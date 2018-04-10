// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "thread.h"
#include "platform.h"

// THUMB is 1 to force Thumb
#ifdef CORTEX_M
#define THUMB 1
#else
#define THUMB 0
#endif

Thread* Thread::_curthread;
Time Thread::_curtimer;
bool Thread::_bootstrapped;

Vector<Thread*> Thread::_runq;

extern SysTimer _systimer;

Time Thread::_qend;
uint Thread::_rr;

uint Thread::_ipl_count;
bool Thread::_pend_csw;


Thread::Thread() :
	_state(State::SLEEP),
    _prio(THREAD_DEFAULT_PRIORITY),
    _waitob(NULL),
    _waittime(0),
	_stack(NULL),
    _estack(NULL) { 
}


Thread* Thread::Create(const char* name, Start func, void* arg, uint stack_size) {
    Thread* t = AllocThreadContext();

    memset(t, 0, sizeof *t);

    ScopedNoInt G;

    Construct(t, func, arg, stack_size);

    t->_name = name;

    // Add to scheduler
    _runq.PushBack(t);

    // Maybe run it
    ContextSwitch();

    return t;
}


void Thread::Construct(Thread* t, Start func, void* arg, uint stack_size) {
    assert(!IntEnabled());

    t->_stack = AllocThreadStack(stack_size);
    t->_estack = (uint8_t*)t->_stack + stack_size;

    t->_state = State::RUN;

    uint32_t *sp = (uint32_t*)t->_estack;

    // Push two zeros for GDB to detect end of frame list
    *--sp = 0;                  // Saved R7
    *--sp = 0;                  // Saved LR

    // XXX this is CM specific
    *--sp = BIT24;                  // Push PSR with thumb bit
    *--sp = (uint32_t)func;         // Push PC
    *--sp = (uint32_t)Thread::Reap+1; // Push LR return address
    *--sp = 0;                      // R12 = 0
    *--sp = 0;                      // R3 = 0
    *--sp = 0;                      // R2 = 0
    *--sp = 0;                      // R1 = 0
    *--sp = (uint32_t)arg;          // R0 = arg
    *--sp = 0;                      // R11 = 0
    *--sp = 0;                      // R10 = 0
    *--sp = 0;                      // R9 = 0
    *--sp = 0;                      // R8 = 0
    *--sp = 0;                      // R7 = 0
    *--sp = 0;                      // R6 = 0
    *--sp = 0;                      // R5 = 0
    *--sp = 0;                      // R4 = 0

    t->_pcb.psp = (uintptr_t)sp;
}


void Thread::Reap() {
    for (;;) {
        _curthread->_state = State::STOP;

        WakeAll(_curthread); // Wake all threads waiting for us

        Idle();             // Should never become runnable again...
    }
}


Thread::~Thread()
{
    ScopedNoInt G;

	assert(_curthread != this);	// Thread can't destroy itself

	const uint pos = _runq.Find(this);
	if (pos != NOT_FOUND)
        _runq.Erase(pos);
}


Thread& Thread::Bootstrap()
{
    assert(!IntEnabled());
    assert(!_curthread);
    assert(!_bootstrapped);
    assert(!_ipl_count);

    _runq.Reserve(8);

    SetCurThread(AllocThreadContext());

    _curthread->_name = "main";
    _curthread->_state = State::RUN;

    _rr = 0;

    _qend = Time::Now() + Time::FromUsec(RRQUANTUM);
    _systimer.SetResolution(TIME_RESOLUTION);

    SetTimer(RRQUANTUM);

	extern void* _main_thread_stack;

    // Bootstrap main thread and interrupt stacks.  The main thread
    // inherits the current stack and the interrupt stack is set up
    // below it.
    //
	// Since the region allocates low to high we do set aside room for
    // the stack by installing a reserve to cover the top portion
    // with the two stacks.

	_iram_region.SetReserve(MAIN_THREAD_STACK + INTR_THREAD_STACK);

	_main_thread_stack = (uint8_t*)_iram_region.GetEnd() - MAIN_THREAD_STACK; // Stack limit

	_curthread->_stack = _main_thread_stack;
	_curthread->_estack = _iram_region.GetEnd();

    // Set up stacks
    StackStrap((uintptr_t)_main_thread_stack - 8);

    _bootstrapped = true;

    // Add to scheduler
    _runq.PushBack(_curthread);

	return *_curthread;
}


void Thread::SetPriority(uint8_t new_prio)
{
    assert(_curthread);

    ScopedNoInt G;
    
	_curthread->_prio = new_prio;

    ContextSwitch();

	assert(!IntEnabled());
}


void Thread::Rotate(bool in_csw)
{
    // If we already have a pending context switch, ignore
    if (!in_csw && _pend_csw)
        return;

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

		if (t == _curthread)
            seencur = true;

		switch (t->_state) {
		case State::TWAIT:
			if (t->_waittime > now) {
				if (!wake || t->_waittime < wake->_waittime)
                    wake = t;
				break;
			}
			t->_state = State::RUN;

			// fallthru
		case State::RUN:
			if (!top || t->_prio > top->_prio)
				top = t;

			if (t->_prio == _curthread->_prio) {
				if (t != _curthread) {
					if (seencur) {
						if (!nextrun)
                            nextrun = t;
					} else {
						if (!firstrun)
                            firstrun = t;
					}
				}
				++numrun;
			}
			break;
		default: ;
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
	} else if (_curthread->_state != State::RUN) {
		// Current thread is blocked, so just pick first runnable, if any
		if (top)
			next = top;
	}

	if (_qend > now)
		SetTimer((min(next_timer, _qend) - now).GetUsec());
	else
		SetTimer((next_timer - now).GetUsec());

	if (next && next != _curthread) {
        if (in_csw) {
            SetCurThread(next);
        } else {
            if (!_ipl_count)
                PostContextSwitch();
            else
                _pend_csw = true;
        }
    } 
}

void Thread::WakeAll(const void* ob)
{
    ScopedNoInt G;

	bool cx = false;
	for (uint i = 0; i < _runq.Size(); ++i) {
		Thread* t = _runq[i];
		
		switch (t->_state) {
		case State::WAIT:
		case State::TWAIT:
			if (t->_waitob == ob)  {
				t->_state = State::RUN;

                if (t->_prio > _curthread->_prio)
                    cx = true;
			}
			break;
		default: ;
		}
	}

	if (cx)
        ContextSwitch();
}


void Thread::WakeSingle(const void* ob)
{
    ScopedNoInt G;

	Thread* top = NULL;
	for (uint i = 0; i < _runq.Size(); ++i) {
		Thread* t = _runq[i];
		
		switch (t->_state) {
		case State::WAIT:
		case State::TWAIT:
			if (t->_waitob == ob && (!top || t->_prio > top->_prio))
				top = t;
			break;
		default: ;
		}
	}
	
	if (top) {
		top->_state = State::RUN;

		if (top->_prio > _curthread->_prio || (_curthread->_state != State::RUN))
            ContextSwitch();
	}
}


void Thread::Idle()  {
	AssertNotInterrupt();

    Rotate(false);

    while (_curthread->_state != State::RUN) {
        const uint prev_ipl = SetIPL(0);
        const uint prev_count = exch<uint>(_ipl_count, 0);

        EnableInterrupts();
        WaitForInterrupt();
        DisableInterrupts();

        _ipl_count = prev_count;
        SetIPL(prev_ipl);
    }

    _curthread->ValidateStack();
}


void Thread::WaitFor(const void* ob)
{
	AssertNotInterrupt();

    ScopedNoInt G;

    _curthread->_state = State::WAIT;
    _curthread->_waitob = ob;

    Idle();
}


void Thread::WaitFor(const void* ob, Time until)
{
	AssertNotInterrupt();

	if (until <= Time::Now())
        return;

    ScopedNoInt G;

    _curthread->_state = State::TWAIT;
    _curthread->_waitob = ob;
    _curthread->_waittime = until;

    Idle();
}


void Thread::Sleep(Time until)
{
	AssertNotInterrupt();

	while (Time::Now() < until) {
        ScopedNoInt G;

		_curthread->_state = State::TWAIT;
		_curthread->_waitob = NULL;
		_curthread->_waittime = until;

        Idle();
	}
}


void Thread::Delay(uint usec) {
    Sleep(Time::Now() + Time::FromUsec(usec)); 
}


void Thread::SetTimer(uint usec)
{
    ScopedNoInt G;

	if (usec < 5)
        usec = 5;
    
	_systimer.SetTimer(min(usec, (uint)1024*1024*4));
}


void Thread::Exception(Thread::ExType ex)
{
    static const char* const cause[6] = {
        "Hard Fault", "Bus Fault", "Usage Fault", "Mem Man Fault", 
        "SVCall", "PENDSV" 
    };

    if (ex >= NUM_EXTYPE) 
        abort();

    panic(cause[ex]);
}


void Thread::ContextSwitchHandler(void*) {
    Rotate(true);
}

void Thread::TimerInterrupt() 
{
    Thread* prev = _curthread;
    Rotate(false);
    if (_curthread != prev)
        ContextSwitch();
}

