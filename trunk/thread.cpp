#include "enetkit.h"
#include "thread.h"
#include "platform.h"
#include "timer.h"


Spinlock Thread::_lock;
Thread* Thread::_curthread;
volatile Thread::Pcb* Thread::_curpcb;
Time Thread::_curtimer;

Vector<Thread*> Thread::_runq(64);

SysTimer _systimer;

Time Thread::_qend;
uint Thread::_rr;


Thread::Thread(void* stack, uint stack_size) :
	_ready(false), _cancel(false),
	_state(STATE_SLEEP), _prio(128), _waitob(NULL), _waittime(0),
	_stack(stack), _estack((uint8_t*)_stack + stack_size)
{ 
	Spinlock::Scoped L(_lock);
	_runq.PushBack(this);
}


Thread::Thread(Start func, void* arg, uint stack_size, bool detached)
{
	new (this) Thread(NULL, stack_size);

	Create(func, arg, stack_size, detached);
}


void Thread::Create(Start func, void* arg, uint stack_size, bool detached)
{
	_lock.Lock();

	if (!_stack) {
		_stack = _stack_region.GetMem(stack_size);
		_estack = (uint8_t*)_stack + stack_size;
	}

	Thread* prev = _curthread;

	if (Suspend()) {
		_curthread = this;
		_curpcb = &_pcb;
		SetStack((uint8_t*)_estack - 4 );
		_state = STATE_RUN;
		_func = func;			// Move these off the stack
		_arg = arg;
		if (Suspend()) Switch(); // Switch to highest-priority thread (may not be this one!)
		_func(_arg);

		// Never proceed past this - if we should become runnable, stop again
		for (;;) {
			_state = STATE_STOP;
			WakeAll(this);		// Wake all threads waiting for us
			if (Suspend()) Switch();
		}
	}

	assert(IntEnabled());
	assert(InSystemMode());

	_lock.Lock();
	_curthread = prev;
	_curpcb = &prev->_pcb;
	_lock.Unlock();
}


Thread::~Thread()
{
	Spinlock::Scoped L(_lock);
	assert(_curthread != this);	// Thread can't destroy itself
	const uint pos = _runq.Find(this);
	if (pos != NOT_FOUND)  _runq.Erase(pos);
}


// * static
Thread& Thread::Initialize()
{
	_lock.Lock();

	assert(!_curthread);
	_curthread = new Thread();
	_curpcb = &_curthread->_pcb;
	_curthread->_state = STATE_RUN;
	_rr = 0;
	_qend = Time::Now() + Time::FromUsec(RRQUANTUM);
//	_systimer.SetResolution(TIME_RESOLUTION);
//	SetTimer(_qend);
	_lock.Unlock();
	_curthread->TakeSnapshot();

	extern void* _main_thread_stack;

	_curthread->_stack = _main_thread_stack;
	_curthread->_estack = (uint8_t*)_main_thread_stack + MAIN_THREAD_STACK;

	return *_curthread;
}


bool Thread::Suspend()
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
		"bic r0, #0x80|0x40;"    // Resume with interrupts enabled
		"str r0, [r1, #16*4-4];"
		"mov r0, #1;"		// Return value
		"str lr, [r1, #15*4-4];"// Save LR as PC so LoadState returns to our caller
		"mov pc, lr"			// Return
		: : : "memory");

	return false;				// Bah.
}


void Thread::Join()
{
	for (;;) {
		_lock.Lock();
		WaitFor(this);
		if (_state == STATE_STOP) break;
	}
}


void Thread::Yield(Thread* other)
{
	_lock.Lock();

	if (other->_state == STATE_STOP) {
		// Ignore attempt to yield to terminated thread
		_lock.Unlock();
		return;
	}

	if (Suspend()) {
		_curpcb = &other->_pcb;
		other->_state = STATE_RUN;
		(_curthread = other)->Resume();
	}
	assert(IntEnabled());
	assert(InSystemMode());
	_lock.Lock();
	_state = STATE_RUN;
	_lock.Unlock();
}


Thread* Thread::Rotate()
{
	const Time now = Time::Now();

	Thread* wake = NULL;		// Next thread on timed wait
	Thread* top = NULL;			// Thread with highest priority
	uint numrun = 0;			// Number of running threads with prio equal to current's
	
	for (uint i = 0; i < _runq.Size(); ++i) {
		Thread* t = _runq[i];

		switch (t->_state) {
		case STATE_RUN:
			if (!top || t->_prio > top->_prio) top = t;
			if (t->_prio == _curthread->_prio) ++numrun;
			break;
		case STATE_TWAIT:
			if (!wake || t->_waittime < wake->_waittime) wake = t;
			break;
		}
	}

	Thread* next = _curthread;

	// If there's a thread with a prio higher than current, switch to it
	if (top && top->_prio > _curthread->_prio) {
		next = top;
	} else if (_curthread->_state != STATE_RUN || (numrun > 1 && now >= _qend)) {
		// If there's nothing running with higher prio and there are multiple threads
		// running with current prio, round-robin.

		// Find next in round-robin cycle
		if (_rr >= _runq.Size() - 1) _rr = 0; else ++_rr;

		next = NULL;
		for (uint i = _rr; i < _runq.Size(); ++i)
			if (_runq[i]->_prio == _curthread->_prio && _runq[i]->_state == STATE_RUN) {
				next = _runq[i];
				break;
			}

		if (!next)
			for (uint i = 0; i < _rr; ++i)
				if (_runq[i]->_prio == _curthread->_prio && _runq[i]->_state == STATE_RUN) {
					next = _runq[i];
					break;
				}
	}

	// Reset end-of-quantum
	// XXX we should only do this if the runq is in such a state that we will need to RR
	_qend = now + Time::FromUsec(RRQUANTUM);

	// Update timer
	if (wake)  SetTimer(min(_qend, wake->_waittime));

	return next;
}


void Thread::Switch()
{
	_lock.AssertLocked();

	do {
		Thread* top = Rotate();

		// If best thread is already current we're done here
		if (top == _curthread) {
			_lock.Unlock();
			return;
		}

		if (top) top->Resume();

		// Do nothing until interrupt.  We'll return when runnable.
		_lock.Unlock();
		WaitForInterrupt();
		_lock.Lock();
	} while (_state != STATE_RUN);
	_lock.Unlock();
}


void Thread::WakeAll(const void* ob)
{
	_lock.Lock();

	bool waiter = false;
	for (uint i = 0; i < _runq.Size(); ++i) {
		Thread* t = _runq[i];
		
		switch (t->_state) {
		case STATE_WAIT:
		case STATE_TWAIT:
			if (t->_waitob == ob)  {
				t->_state = STATE_RUN;
				waiter = true;
			}
			break;
		}
	}

	if (waiter) {
		if (Suspend()) Switch();
	} else
		_lock.Unlock();
}


void Thread::WakeSingle(const void* ob)
{
	_lock.Lock();

	Thread* top = NULL;
	for (uint i = 0; i < _runq.Size(); ++i) {
		Thread* t = _runq[i];
		
		switch (t->_state) {
		case STATE_WAIT:
		case STATE_TWAIT:
			if (t->_waitob == ob && (!top || t->_prio > top->_prio))
				top = t;
			break;
		}
	}
	
	if (top) top->_state = STATE_RUN;

	if (top && top->_prio > _prio) {
		if (Suspend()) Switch();
	} else
		_lock.Unlock();
}


void Thread::TimerInterrupt()
{
	Spinlock::Scoped L(_lock);

	// Wake waiting threads
	const Time now = Time::Now();
	for (uint i = 0; i < _runq.Size(); +i) {
		Thread* t = _runq[i];
		if (t->_state == STATE_TWAIT && t->_waittime <= now) t->_state = STATE_RUN;
	}

	// Perform runq rotation
	Thread* t = Rotate();
	if (t) {
		_curthread = t;
		_curpcb = &t->_pcb;
	}
}


void Thread::WaitFor(const void* ob)
{
	_lock.Lock();
	_state = STATE_WAIT;
	_waitob = ob;
	if (Suspend()) Switch();
}


void Thread::WaitFor(const void* ob, Time until)
{
	if (until <= Time::Now()) {
		WaitFor(ob);
		return;
	}

	_lock.Lock();
	_state = STATE_TWAIT;
	_waitob = ob;
	_waittime = until;

	if (Suspend()) Switch();
}


void Thread::Sleep(Time until)
{
	while (Time::Now() < until) {
		_lock.Lock();
		_state = STATE_TWAIT;
		_waitob = NULL;
		_waittime = until;
		if (Suspend()) Switch();
	}
}


void Thread::Delay(uint usec) { Sleep(Time::Now() + Time::FromUsec(usec)); }


void Thread::SetTimer(Time deadline)
{
	Spinlock::Scoped L(_lock);

	if (deadline != _curtimer) {
		_curtimer = deadline;
		_systimer.RunTimer(deadline.GetUsec(), false);
	}
}


// * virtual
void SysTimer::Tick()
{
	Thread::TimerInterrupt();
}
