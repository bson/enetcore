// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __THREAD_H__
#define __THREAD_H__

#include "vector.h"
#include "arithmetic.h"

typedef class Thread* ThreadId;

// These are implemented by board specific code to manage memory layout
uint8_t* AllocThreadStack(uint size);
Thread* AllocThreadContext();

class Thread {
public:
	typedef void* (*Start)(void*);

private:
    // Currently running thread
	static Thread* _curthread;

    // True if main thread has bootstrapped
    static bool _bootstrapped;

	// Run queue - because of the small number of threads we keep only a single table
	static Vector<Thread*> _runq;

	static uint _rr;	// Round robin index
	static Time _qend;  // End of current quantum

	enum { RRQUANTUM = 20*1024 }; // Round robin quantum, in usec

private:
	// Thread execution state
	enum class State: uint8_t {
		RUN = 0,			// Running
		WAIT,				// Waiting on _waitob
		TWAIT,              // In timed wait until _waitob or _waittmr
		SLEEP,              // In sleep
        STOP                // Stopped
	};

    // These items are at fixed offsets into this structure.
    PcbPrimitive _pcb;          // [0x0]
    const char* _name;          // Thread identifier string [0x4]
	volatile State _state;      // Thread state [0x8]
	uint8_t _prio;		 		// Thread priority - lowest is 0, highest 255 [0x9]
    // End of fixed offsets.

	const void* _waitob;		// Object thread is blocked on, if any
	Time _waittime;				// Absolute wake time when in STATE_TWAIT

	void* _stack;				// Beginning of stack memory block (low address)
	void* _estack;				// End of stack (high address - first address past the stack)

protected:
    friend class IPL;

    static uint _ipl_count;     // IPL nesting count
    static bool _pend_csw;    // need context switch that was blocked by IPL lockout

public:
	Thread();
	~Thread();

	// Initialize Thread system - must be called from main thread.
	// Returns its Thread.
	static Thread& Bootstrap();

    static bool Bootstrapped() { return _bootstrapped; }

    // Create new thread
    static Thread* Create(const char* name, Start func, void *arg, 
                          uint stack_size = THREAD_DEFAULT_STACK);

	// Can be used to check for stack overruns
	void ValidateStack()  {
#ifdef DEBUG
		void* sp; GetSP(sp);
		assert(sp > _stack);
		assert(sp <= _estack);
#endif
	}

	// Change thread priority
	static void SetPriority(uint8_t new_prio);

	// Wake all threads, if any, waiting for an object.
	static void WakeAll(const void* ob);

	// Wake highest priority thread, if any, waiting for an object
	static void WakeSingle(const void* ob);

	// Wait for an object.
	static void WaitFor(const void* ob);
	static void WaitFor(const void* ob, Time until); // until = absolute time

	// Sleep
	static void Delay(uint usec);
	static void Sleep(Time until);

	// Thread exception handler.  _curpcb should contain thread state
	enum ExType { 
        HARD_FAULT = 0,
        BUS_FAULT,
        USAGE_FAULT,
        MEM_MAN_FAULT,
        SVCALL,
        PENDSV,
        NUM_EXTYPE
    };

	[[noreturn]] static void Exception(ExType ex);

    // Request a context switch.
    static void ContextSwitch() {
        if (!InExceptionHandler()) {
            assert(!IntEnabled());
            PostContextSwitch();
            EnableInterrupts(); // Permit the switch exception to happen
            DisableInterrupts();
        } else {
            PostContextSwitch();
        }
    }

    // For various housekeeping purposes
    static Thread* GetCurThread() { return _curthread; }

    // Context switch, called form exception handler
    static void ContextSwitchHandler(void*);

    // Timer interrupt 
    static void TimerInterrupt();

public:
    // IPL management.  This is an interruptible alternative to
    // disabling interrupts.  It can be interrupted by any higher IPL.
    // While any thread is running with an IPL restriction context
    // switches are disabled, but interrupts generally remain enabled.
    // Note that calling WaitFor() will automatically drop the IPL and
    // enabled interrupts during the wait, then restore the IPL and
    // interrupt enable mask on wake.  Hence, while a thread is
    // blocked interrupts are wide open as are context switches.  If
    // Rotate() is called and it decides a context switch is needed,
    // with an elevated IPL it will set the flag _pend_csw instead
    // of performing the switch.  When the thread holding the IPL
    // lockout releases it, the flag is checked and any pending
    // switch posted.
    class IPL {
        uint32_t _save;
    public:
        IPL(uint32_t ipl)
            : _save(SetIPL(ipl - 1)) {
            __atomic_inc(&Thread::_ipl_count);
        }
        
        ~IPL() {
            SetIPL(_save);
            if (!__atomic_dec(&Thread::_ipl_count) && Thread::_pend_csw)
                ContextSwitch();
        }

    private:
        IPL(const IPL&);
        IPL& operator=(const IPL&);
    };

private:
    // Idle - wait to become runnable
    static void Idle();

    // where threads go to die
    static void Reap();

    // Wrapper to change current thread. Guarantees PCB ptr stays in sync.
    static void SetCurThread(Thread* t) {
        assert(!IntEnabled() || InExceptionHandler());
        _curthread = t;
    }

    // Construct new thread.  Must be called with interrupts disabled.
	static void Construct(Thread *t, Start func, void *arg,
                          uint stack_size = THREAD_DEFAULT_STACK);

    // Just static asserts.  These need to be in a dummy function because
    // the offsets are not known until the closing brace, and the members
    // are private.
    static void StaticAssert();

public:

	// See if the scheduler wants to run something else.
	static void Rotate(bool in_csw);

	// Set timer
	static Time _curtimer; // Current timer setting
	static void SetTimer(uint usec);  // usec from now
};

[[__finline]] inline void Thread::StaticAssert() {
    static_assert(offsetof(Thread, _pcb) == 0);
    static_assert(offsetof(Thread, _name) == sizeof(void*));
    static_assert(offsetof(Thread, _state) == sizeof(void*)*2);
    static_assert(offsetof(Thread, _prio) == sizeof(void*)*2 + 1);
}

extern Thread* _main_thread;

#endif // __THREAD_H__
