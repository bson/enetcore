#ifndef __THREAD_H__
#define __THREAD_H__

typedef class Thread* ThreadId;

class Thread {	
public:
	typedef void* (*Start)(void*);

	struct TLS { };

private:
	static Spinlock _lock;		// Global thread lock
	static Thread* _curthread;	// Currently running thread

	volatile bool _ready:1;		// Initialized
	volatile bool _cancel:1;	// Cancel

	Start _func;				// Start function
	void* _arg;					// Arg to start function

	// Thread TLS area
	TLS _tls;

	typedef PcbPrimitive Pcb;
	volatile Pcb _pcb;
	static volatile Pcb* _curpcb asm("__curpcb"); // PCB of currently running thread

	// Run queue - because of the small number of threads we keep only a single table
	static Vector<Thread*> _runq;

	static uint _rr;			// Round robin index
	static Time _qend;			// End of current quantum

	enum { RRQUANTUM = 20*1024 };		// Round robin quantum, in usec

	// Thread execution state
	enum State {
		STATE_RUN = 0,			// Running
		STATE_RESUME,			// Resuming
		STATE_WAIT,				// Waiting on _waitob
		STATE_TWAIT,			// In timed wait until _waitob or _waittmr
		STATE_SLEEP,			// In indefinite sleep
		STATE_STOP				// Stopped (waiting for join)
	};

	State _state;
	uint8_t _prio;		 		// Thread priority - lowest is 0, highest 255
	const void* _waitob;		// Object thread is blocked on, if any
	Time _waittime;				// Absolute wake time when in STATE_TWAIT

	void* _stack;				// Beginning of stack memory block (low address)
	void* _estack;				// End of stack (high address - first address past the stack)

public:
	Thread(void* stack = NULL, uint stack_size = 0);
	~Thread();

	// Same as Thread t; t.Create(func, arg); t.WaitForReady();
	Thread(Start func, void* arg, uint stack_size = THREAD_DEFAULT_STACK,
		   bool detached = false);

	// Initialize Thread system - must be called from main thread.  Returns its Thread.
	static Thread& Initialize();

	static __force_inline Thread& Self() { return *_curthread; }

	void Cancel() { _cancel = true; }
	static __force_inline bool IsCanceled()  { return Self()._cancel; }

	void Join();

	void Create(Start func, void *arg, uint stack_size = THREAD_DEFAULT_STACK,
				bool detached = false);
	static void ReadyToWork() { Self()._ready = true;  WakeAll(&Self()); }
	void WaitForReady()  { while(!_ready) WaitFor(this); }

	static TLS& GetTLS() { return Self()._tls; }

	// Can be used to check for stack overruns
	void ValidateStack()  {
#ifdef DEBUG
		void* sp; GetSP(sp);
		assert(sp < _estack);
		assert(sp >= _stack);
#endif
	}

	// Change thread priority
	void SetPriority(uint8_t new_prio);

 	// Explicitly yield to specific thread
	void Yield(Thread* other);

	// Wake all threads, if any, waiting for an object.
	static void WakeAll(const void* ob);

	// Wake highest priority thread, if any, waiting for an object
	static void WakeSingle(const void* ob);

	// Wait for an object.
	void WaitFor(const void* ob);
	void WaitFor(const void* ob, Time until); // until = absolute time

	// Sleep
	void Delay(uint usec);
	void Sleep(Time until);

	// Timer interrupt entry
	static void TimerInterrupt();

	// Thread exception handler.  _curpcb should contain thread state
	enum ExType { DATA_ABORT = 0, PROGRAM_ABORT, UNDEF, SWI };
	static void Exception(ExType ex) __naked __noreturn;

private:
	// Save/resume state of self - this function will return after save, then
	// again after Resume().  The first time (immediately), it returns true, the
	// second time (after Resume()) false.  _lock must be held on entry and will
	// be held across (unbroken) on the first return, but will not be held on the
	// second.  No other spin lock must be held while suspending a thread.
	//
	// _lock must be held on Resume();
	//
	// Canonical use of Suspend-Resume:
	//
	//    _lock.Lock();
	//    // ... prepare ...
	//    if (Suspend()) {
	//       // ... do work and figure out which thread to run next  ...
	//       other_thread->Resume();
	//       // not reached
	//    }
	//
	//    // _lock is no longer held here.  The thread is fully pre-emptible after
	//    // this thread has been resumed. If _lock is needed, it must be reacquired.
	//
	static bool Suspend() __naked;
	static void Resume() { ThreadResumePrimitive(); }

	// Change currently running thread's stack and start new frame chain
	// Also set up stack limit.  end is lowest addr (i.e. start of region).
	static void __force_inline SetStack(void* end, void* new_stack) {
		_lock.AssertLocked();
		SetThreadStackPrimitive(end, new_stack);
	}

	// Take snapshot of current thread state.
	// This is mainly for diagnostic purposes
	void TakeSnapshot()  {
#ifdef DEBUG
		_lock.Lock();
		if (Suspend()) Resume();
		_lock.Lock();
		_state = STATE_RUN;
		_lock.Unlock();
#endif
	}

public:

	// Perform thread rotation.  This function does all except actually
	// resume the updated _curthread.  The reason for this is that the
	// resume mechanism differs depending on context (exception vs system).
	// Switch() wraps this function for use from system mode.
	// TimerInterrupt() calls it directly from exception mode.
	static void Rotate();

	// Perform thread rotation - from system context.
	// Called with _lock held, returns without.
	static void Switch();

	// Set timer
	static Time _curtimer;				// Current timer setting
	static void SetTimer(uint usec);	// usec from now
};

__force_inline Thread& Self() { return Thread::Self(); }

extern Thread* _main_thread;

#endif // __THREAD_H__
