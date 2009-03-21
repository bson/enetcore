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

//	mutable WaitQ _waitq;		// Signals Thread state change

	volatile bool _ready:1;		// Initialized
	volatile bool _cancel:1;	// Cancel
	volatile bool _exit:1;		// Has exited

	// Thread TLS area
	TLS _tls;

	struct Pcb {
		uint32_t _regs[17];		// R0-R15, CPSR
	};

	volatile Pcb _pcb;
	static volatile struct Pcb* _curpcb asm("__curpcb"); // PCB of currently running thread

#ifdef DEBUG
	void* _stack;				// Beginning of stack memory block (low address)
	void* _estack;				// End of stack (high address - first address past the stack)
#endif

public:
	Thread(void* stack = 0, uint stack_size = 0) :
		_ready(false), _cancel(false), _exit(false),
		_stack(stack), _estack((uint8_t*)_stack + stack_size)
	{ }
	~Thread() { }

	// Same as Thread t; t.Create(func, arg); t.WaitForReady();
	Thread(Start func, void* arg, uint stack_size = THREAD_DEFAULT_STACK,
		   bool detached = false);

	// Initialize Thread system - must be called from main thread.  Returns its Thread.
	static Thread& Initialize();

	static INLINE_ALWAYS Thread& Self() { return *_curthread; }

	void Cancel() { _cancel = true; }
	static INLINE_ALWAYS bool IsCanceled()  { return Self()._cancel; }

	void Join() { /* while (!_exit)  _waitq.Wait();*/ }

	void Create(Start func, void *arg, uint stack_size = THREAD_DEFAULT_STACK,
				bool detached = false);
	static void ReadyToWork() { Self()._ready = true; /* Self()._waitq.WakeAll(); */ }
	void WaitForReady() volatile const { /* while(!_read)  _waitq.Wait(); */ }

	static TLS& GetTLS() { return Self()._tls; }

	// Can be used to check for stack overruns
	void ValidateStack()  {
#ifdef DEBUG
		void* sp; asm volatile ("mov %0, sp" : "=r"(sp) : : "memory");
		assert(sp < _estack);
		assert(sp >= _stack);
#endif
	}

 	// Yield to other thread
	void Yield(Thread* other) {
		_lock.Lock();
		if (Suspend()) {
			_curpcb = &other->_pcb;
			(_curthread = other)->Resume();
		}
		assert(IntEnabled());
		assert(InSystemMode());
	}

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
	// This function is not MP safe as implemented.
	
	bool NAKED Suspend() volatile;
	void INLINE_ALWAYS Resume() volatile {
		_lock.Abandon();
		_pcb._regs[0] = 0;		// Return 0 in R0: return value from Suspend() after Resume()
		asm volatile (
			"mov  r0, %0;"		   // &_pcb
			"ldr r1, [r0, #16*4];" // R1 = saved PSR
			"msr cpsr, r1;"		   // CPSR = saved PSR
			"ldm r0, {r0-r15};"	   // Load saved R0-R14,PC, CPSR=SPSR
			: : "r"(&_pcb) : "memory", "cc");
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
			"mrs r0, cpsr;"											\
			"bic r0, #0x40;" /* Rerenable FIQ */					\
			"msr cpsr, r0;"											\
			: : : "memory"); }


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

};

extern Thread* _main_thread;

#endif // __THREAD_H__
