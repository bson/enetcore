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
	_state(STATE_SLEEP), _prio(THREAD_DEFAULT_PRIORITY), _waitob(NULL), _waittime(0),
	_stack(stack), _estack((uint8_t*)_stack + stack_size)
{ 
	Spinlock::Scoped L(_lock);
	_runq.PushBack(this);
}


Thread::Thread(Start func, void* arg, uint stack_size, bool detached)
{
	new (this) Thread(NULL, stack_size);

	Create(func, arg, stack_size, detached);
//	WaitForReady();
}


void Thread::Create(Start func, void* arg, uint stack_size, bool detached)
{
	_lock.Lock();

	if (!_stack) {
		_stack = _stack_region.GetMem(stack_size);
		_estack = (uint8_t*)_stack + stack_size;
	}

	if (Suspend()) {
		_curthread = this;
		_curpcb = &_pcb;
		SetStack((uint8_t*)_estack - 4);
		_state = STATE_RUN;
		_func = func;			// Move these off the stack
		_arg = arg;
		if (Suspend()) Switch(); // Switch to highest-priority thread (may not be this one!)

		_lock.Lock();
		_state = STATE_RUN;
		_lock.Unlock();

		_func(_arg);

		// Never proceed past this - if we should become runnable, stop again
		for (;;) {
			_state = STATE_STOP;
			WakeAll(this);		// Wake all threads waiting for us
			if (Suspend()) Switch();
			_lock.Lock();
			_state = STATE_STOP;
			_lock.Unlock();

		}
	}

	assert(IntEnabled());
	assert(InSystemMode());

	_lock.Lock();
	_curthread->_state = STATE_RUN;
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
	_systimer.SetResolution(TIME_RESOLUTION);
	SetTimer(RRQUANTUM);
	_lock.Unlock();
	_curthread->TakeSnapshot();

	extern void* _main_thread_stack;

	_curthread->_stack = _main_thread_stack;
	_curthread->_estack = (uint8_t*)_main_thread_stack + MAIN_THREAD_STACK;

	return *_curthread;
}


void Thread::SetPriority(uint8_t new_prio)
{
	_lock.Lock();
	_prio = new_prio;
	if (Suspend()) Switch();
	_lock.Lock();
	_state = STATE_RUN;
	_lock.Unlock();
}


bool Thread::Suspend()
{
	asm volatile (
		"ldr r0, =__curpcb;"
		"ldr r0, [r0];"
		"add r0, r0, #4;"
		"stm r0, {r1-r14};"
		"mov r1, #0;"			// Return value after Resume
		"str r1, [r0, #-4];"
		"mrs r1, cpsr;"
		"bic r1, r1, #0x80|0x40;"    // Resume with interrupts enabled
		"str r1, [r0, #16*4-4];"
		"str lr, [r0, #15*4-4];"// Save LR as PC so LoadState returns to our caller
		"mov r0, #1;"			// Return value now
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
		other->_state = STATE_RUN;
		_curpcb = &other->_pcb;
		_curthread = other;
		Resume();
	}
	assert(IntEnabled());
	assert(InSystemMode());
	_lock.Lock();
	_state = STATE_RUN;
	_lock.Unlock();
}


// * static
void Thread::Rotate()
{
	_lock.AssertLocked();

	const Time now = Time::Now();

	// Make a pass through runq, collecting the parameters we'll need

	Thread* wake = NULL;		// Next thread due on timed wait
	Thread* top = NULL;			// Thread with highest priority
	uint numrun = 0;			// Number of running threads with prio equal to current's
	Thread* firstrun = NULL;	// First other thread with prio equal to current
	Thread* nextrun = NULL;		// Next other thread with prio equal to current
	bool seencur = false;		// Set to true when we've encountered current thread
	
	for (uint i = 0; i < _runq.Size(); ++i) {
		Thread* t = _runq[i];

		if (t == _curthread) seencur = true;

		switch (t->_state) {
		case STATE_TWAIT:
			if (t->_waittime > now) {
				if (!wake || t->_waittime < wake->_waittime) wake = t;
				break;
			}
			t->_state = STATE_RUN;

			// fallthru
		case STATE_RUN:
			if (!top || t->_prio > top->_prio) {
				top = t;
			}
			if (t->_prio == _curthread->_prio) {
				if (t != _curthread) {
					if (seencur) {
						if (!nextrun) nextrun = t;
					} else {
						if (!firstrun) firstrun = t;
					}
				}
				++numrun;
			}
			break;
		}
	}

	Thread* next = _curthread;
	Time next_timer = wake ? wake->_waittime : now + Time::FromSec(10); // 10sec is a gratuitous upper bound

	// If there's a thread with a prio higher than current, switch to it
	if (top->_prio > _curthread->_prio) {
		next = top;
	} else if (numrun > 1 && now >= _qend) {
		// If there's nothing running with higher prio and there are multiple threads
		// running with current prio, round-robin when the quantum is over.

		assert(nextrun || firstrun);
		next = nextrun ? nextrun : firstrun;
		assert(next != _curthread);

		_qend = now + Time::FromUsec(RRQUANTUM);
	} else if (_curthread->_state != STATE_RUN && _curthread->_state != STATE_RESUME) {
		// Current thread is blocked, so just pick first runnable, if any
		if (top)
			next = top;
	}

	if (_qend > now)
		SetTimer((min(next_timer, _qend) - now).GetUsec());
	else
		SetTimer((next_timer - now).GetUsec());

	if (next) {
		_curthread = next;
		_curpcb = &next->_pcb;
	}
}


// * static
void Thread::Switch()
{
	_lock.AssertLocked();

	Rotate();

	if (_curthread->_state == STATE_RUN)
		Resume();

	// Nothing runnable - idle
	while (_curthread->_state != STATE_RESUME && _curthread->_state != STATE_RUN) {
		_lock.Unlock();
#ifdef USE_IDLE
		// This is incompatible with JTAG since it stops the CPU
		WaitForInterrupt();
#endif
		_lock.Lock();
	}
	_curthread->_state = STATE_RUN;
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
		if (InSystemMode()) {
			if (Suspend()) Switch();
			_lock.Lock();
			_curthread->_state = STATE_RUN;
		} else {
			Rotate();
		}
	}
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
	
	if (top) {
		top->_state = STATE_RUN;

		if (top->_prio > _curthread->_prio ||
			(_curthread->_state != STATE_RUN && _curthread->_state != STATE_RESUME)) {
			if (InSystemMode()) {
				if (Suspend()) Switch();
				_lock.Lock();
				_curthread->_state = STATE_RUN;
			} else {
				Rotate();
			}
		}
	}
	_lock.Unlock();
}


void Thread::TimerInterrupt()
{
	Spinlock::Scoped L(_lock);

	Rotate();
}


void Thread::WaitFor(const void* ob)
{
	_lock.Lock();
	_state = STATE_WAIT;
	_waitob = ob;
	if (Suspend()) Switch();
	_lock.Lock();
	_state = STATE_RUN;
	_lock.Unlock();
}


void Thread::WaitFor(const void* ob, Time until)
{
	if (until <= Time::Now()) return;

	_lock.Lock();
	_state = STATE_TWAIT;
	_waitob = ob;
	_waittime = until;

	if (Suspend()) Switch();
	_lock.Lock();
	_state = STATE_RUN;
	_lock.Unlock();
}


void Thread::Sleep(Time until)
{
	while (Time::Now() < until) {
		_lock.Lock();
		_state = STATE_TWAIT;
		_waitob = NULL;
		_waittime = until;
		if (Suspend()) Switch();
		_lock.Lock();
		_state = STATE_RUN;
		_lock.Unlock();
	}
}


void Thread::Delay(uint usec) { Sleep(Time::Now() + Time::FromUsec(usec)); }


void Thread::SetTimer(uint usec)
{
	Spinlock::Scoped L(_lock);

	if (usec < 20) usec = 20;
	_systimer.SetTimer(min(usec, (uint)1024*1024*4));
//	}
}


// * static
void Thread::Exception(Thread::ExType ex)
{
	switch (ex) {
	case DATA_ABORT:
		panic("Data abort");
	case PROGRAM_ABORT:
		panic("Program abort");
	case UNDEF:
		panic("Undef instruction");
	case SWI:
		panic("SWI exception");
	default:
		abort();
	}
}


// * virtual
void SysTimer::Tick()
{
	Thread::TimerInterrupt();
}
