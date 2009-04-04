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

	struct Pcb {
		uint32_t _regs[17];		// R0-R15, CPSR
		uint32_t _flags;		// Flag word: bit = 0 do not save current thread on exception
	};

	volatile Pcb _pcb;
	static volatile struct Pcb* _curpcb asm("__curpcb"); // PCB of currently running thread

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

	static INLINE_ALWAYS Thread& Self() { return *_curthread; }

	void Cancel() { _cancel = true; }
	static INLINE_ALWAYS bool IsCanceled()  { return Self()._cancel; }

	void Join();

	void Create(Start func, void *arg, uint stack_size = THREAD_DEFAULT_STACK,
				bool detached = false);
	static void ReadyToWork() { Self()._ready = true;  WakeAll(&Self()); }
	void WaitForReady()  { while(!_ready) WaitFor(this); }

	static TLS& GetTLS() { return Self()._tls; }

	// Can be used to check for stack overruns
	void ValidateStack()  {
#ifdef DEBUG
		void* sp; asm volatile ("mov %0, sp" : "=r"(sp) : : "memory");
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
	static void Exception(ExType ex) NAKED NORETURN;

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
	static bool NAKED Suspend();
	static void Resume() {
		_curthread->_state = STATE_RESUME;	// Keep other CPUs from racing to resume
		_curpcb = &_curthread->_pcb;
		_lock.Abandon();
		asm volatile (
			"mov  r0, %0;"		   // &_pcb
			"ldr r1, [r0, #16*4];" // R1 = saved PSR
			"msr cpsr, r1;"		   // CPSR = saved PSR
			"ldm r0, {r0-r15};"	   // Load saved R0-R14,PC, CPSR=SPSR
			: : "r"(_curpcb) : "memory", "cc");
	}


	// Change currently running thread's stack
	static void INLINE_ALWAYS SetStack(void* new_stack) {
		_lock.AssertLocked();
		asm volatile ("mov sp, %0" : : "r"(new_stack) : "memory");
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
	// Save state on exception that may cause a thread switch.
	// The exception handler must be declared NAKED so it has no implicit
	// prologue-epilogue.
	// OFFSET is how much LR deviates from the return location:
	// 4 for IRQ/FIQ, 8 for Abort/Undef.
#define SaveStateExc(OFFSET)									\
	{	asm volatile(											\
			"str r1, [sp,#-4]!;"								\
			"ldr r1, =__curpcb;"								\
			"ldr r1, [r1];"										\
			"str r0, [r1], #4;"									\
			"ldr r0, [sp];"										\
			"str r0, [r1], #4;"									\
			"stm r1, {r2-r14}^;"								\
			"sub lr, lr, #" #OFFSET ";"							\
			"str lr, [r1, #13*4]!;"  /* Save pre-exception PC as PC */ \
			"mrs r0, spsr;"											\
			"str r0, [r1, #4];" /* Save SPSR as CPSR */				\
			: : : "memory"); }

#if 0
// XXX IRQs and exceptions don't disable FIQ
			"mrs r0, cpsr;"											\
			"bic r0, #0x40;" /* Enable FIQ */					\
			"msr cpsr, r0;"											\ //
#endif


	// Load state - return from exception
#define LoadStateReturnExc() 											\
		{ asm volatile(													\
			"add sp, sp, #4; "											\
			"ldr r0, =__curpcb;"										\
			"ldr r0, [r0];"												\
			"ldr r1, [r0, #16*4];" /* R1 = saved PSR */					\
			"msr spsr, r1;"		   /* SPSR = saved PSR */				\
			"ldr lr, [r0,#15*4];"  /* LR_irq = saved PC */				\
			"ldm r0, {r0-r14}^; "   /* Load saved user R0-R14 */		\
			"movs pc, lr"			/* Return */						\
			: : : "memory"); }

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

extern Thread* _main_thread;

#endif // __THREAD_H__
