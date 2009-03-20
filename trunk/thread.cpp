#include "enetkit.h"
#include "thread.h"
#include "platform.h"


Spinlock Thread::_lock;
Thread* Thread::_curthread;
volatile Thread::Pcb* Thread::_curpcb;


Thread::Thread(Start func, void* arg, uint stack_size, bool detached) :
	_ready(false), _cancel(false), _exit(false), _stack(NULL)
{
	Create(func, arg, stack_size, detached);
}


void Thread::Create(Start func, void* arg, uint stack_size, bool)
{
	_lock.Lock();

	if (!_stack) {
		_stack = _stack_region.GetMem(stack_size);
		_estack = (uint8_t*)_stack + stack_size;
	}

	Thread* prev = _curthread;

	_lock.Unlock();
	if (SaveState()) {
		_lock.Lock();
		_curthread = this;
		_curpcb = &_pcb;
		SetStack((uint8_t*)_estack - 4 );
		_lock.Unlock();
		func(arg);
		_exit = true;
		// _waitq.WakeAll()
		// Won't return here, because join() will have reaped thread
		panic("coop thread exit");
	}
	_lock.Lock();
	_curthread = prev;
	_curpcb = &prev->_pcb;
	_lock.Unlock();
}


// * static
Thread& Thread::Initialize()
{
	_lock.Lock();

	assert(!_curthread);
	_curthread = new Thread();
	_curpcb = &_curthread->_pcb;
	_lock.Unlock();
	_curthread->TakeSnapshot();

	extern void* _main_thread_stack;

	_curthread->_stack = _main_thread_stack;
	_curthread->_estack = (uint8_t*)_main_thread_stack + MAIN_THREAD_STACK;

	return *_curthread;
}


bool Thread::SaveState() volatile
{
	asm volatile (
		"str r0, [sp,#-4];"
		"ldr r0, =__curpcb;"
		"ldr r0, [r0];"
		"add r0, r0, #4;"
		"stm r0, {r1-r14};"
		"mov r1, r0;"
		"ldr r0, [sp, #-4];"
		"str r0, [r1, #-4];"
		"mrs r0, cpsr;"
		"str r0, [r1, #16*4-4];"
		"mov r0, #1;"		// Return value
		"str lr, [r1, #15*4-4];"// Save LR as PC so LoadState returns to our caller
		"mov pc, lr"			// Return
		: : : "memory", "r0", "r1", "cc");

	return false;				// Bah.
}
