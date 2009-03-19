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
		if (SaveState()) {
			_curpcb = &other->_pcb;
			(_curthread = other)->LoadState();
		}
	}

private:
	// Save state of self - this function will return after save, then
	// again after LoadState().  The first time, it returns true, the
	// second time (after LoadState()) false.
	bool NAKED SaveState() volatile;

	// Save state in PCB, but make thread return to specific context
	// on return (used by interrupt handler).  The LR needs to be
	// normalized to the actual return location (e.g. by -4 for
	// interrupt, -8 for data abort).  This only returns once.
	void NAKED SaveState(uint32_t lr, uint32_t spsr) volatile;

	// Explicit return (from SaveState, which will return 0)
	void INLINE_ALWAYS LoadState() volatile {
		DisableInterrupts();
		_pcb._regs[0] = 0;		// Return 0 in R0
		asm volatile (
			"mov  r0, %0;"		   // &_pcb
			"ldr r1, [r0, #16*4];" // R1 = saved PSR
			"msr cpsr, r1;"		   // CPSR = saved PSR
			"ldm r0, {r0-r15};"	   // Load saved R0-R14,PC, CPSR=SPSR
			: : "r"(&_pcb) : "memory", "cc");
	}


	// Load state - return from exception (non-user/system mode)
	void INLINE_ALWAYS LoadStateExc() volatile {
		asm volatile (
			"mov  r0, %0;"		   // &_pcb
			"ldr r0, [r0, #16*4];" // R0 = saved PSR
			"msr spsr, r0;"		   // SPSR = saved PSR
			"ldm r0, {r0-r15}^;"   // Load saved R0-R14,PC, CPSR=SPSR
			: : "r"(&_pcb) : "memory", "cc");
	}

	// Clone into other thread
	// Returns true in cloner, false in clonee
	// Currently all that's done is SaveState(), but in the future will plumb itself into the scheduler
	bool Clone() { return SaveState(); }

	// Change thread's stack
	static void INLINE_ALWAYS SetStack(void* new_stack) {
		_lock.AssertLocked();
		_curpcb->_regs[13] = (uint32_t)new_stack;
		asm volatile ("mov sp, %0" : : "r"(new_stack) : "memory");
	}

	// Take snapshot of current thread state.
	// This is mainly for diagnostic purposes
	void TakeSnapshot()  {
#ifdef DEBUG
		if (SaveState()) LoadState();
#endif
	}
};

extern Thread* _main_thread;

#endif // __THREAD_H__
