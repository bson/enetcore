#ifndef __ARM_H__
#define __ARM_H__


// Generic ARM code and definitions

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define BYTE_ORDER LITTLE_ENDIAN


#ifdef __GNUC__
#define __irq   __attribute__((interrupt("IRQ"))) __noinstrument
#define __fiq   __attribute__((interrupt("FIQ"))) __noinstrument
#define __abort   __attribute__((interrupt("ABT"))) __noinstrument
#define __undef   __attribute__((interrupt("UNDEF"))) __noinstrument
#define __swi   __attribute__((interrupt("SWI"))) __noinstrument
#else
#error "Unsupported compiler"
#endif


typedef void (*IRQHandler)();


#ifdef __GNUC__
uint DisableInterrupts() __finline;
inline uint DisableInterrupts() {
	uint prev;
	asm volatile("mrs r12, cpsr\n"
				 "mov %0, r12\n"
				 "orr r12, #0x80\n"
				 "msr cpsr, r12"
				 : "=r" (prev) : : "r12", "cc", "memory");
	return prev;
}

void EnableInterrupts(uint prev) __finline;
inline void EnableInterrupts(uint prev) {
	asm volatile("msr cpsr, %0" : : "r" (prev) : "cc", "memory");
}

// Test if interrupts are enabled
inline bool IntEnabled() {
	uint32_t cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr) : : );
	return !(cpsr & 0x80);
}


// Test if in user/system more (if not, we're in an exception mode)
inline bool InSystemMode() {
	uint32_t cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr) : : );
	return (cpsr & 0x1f) == 0x1f;
}
#else
#error "Unsupported compiler"
#endif



//// Thread support

// Structure included by Thread to use to store context
struct PcbPrimitive {
	uint32_t _regs[17];		// R0-R15, CPSR
	uint32_t _flags;		// Flag word: bit = 0 do not save current thread on exception
};


#if defined (__GNUC__)

// The following stuff is all #define's because they need to be
// inlined _verbatim_ in a specific environment.  A potential function
// call or other overhead in around of these blocks of code could
// cause serious damage.

#define GetSP(SP)															\
	asm volatile ("mov %0, sp" : "=r"(SP) : : "memory")


// See Thread::SetStack()
#define SetThreadStackPrimitive(LIMIT, NEW) \
	asm volatile ("mov sp, %0; mov fp, #0" : : "r"(NEW) : "memory")

// See Thread::Suspend()
#define ThreadSuspendPrimitive()										\
	asm volatile (														\
		"ldr r0, =__curpcb;"											\
		"ldr r0, [r0];"													\
		"add r0, r0, #4;"												\
		"stm r0, {r1-r14};"												\
		"mov r1, #0;"			/* Return value after Resume */			\
		"str r1, [r0, #-4];"											\
		"mrs r1, cpsr;"													\
		"bic r1, r1, #0x80|0x40;"    /* Resume with interrupts enabled */ \
		"str r1, [r0, #16*4-4];"										\
		"str lr, [r0, #15*4-4];"/* Save LR as PC so LoadState returns to our caller */ \
		"mov r0, #1;"			/* Return value now */					\
		"mov pc, lr"			/* Return */							\
		: : : "memory")

// See Thread::Resume()
#define ThreadResumePrimitive()											\
	_curthread->_state = STATE_RESUME;	/* Keep other CPUs from racing to resume */ \
	_curpcb = &_curthread->_pcb;										\
	_lock.Abandon();													\
	asm volatile (														\
		"mov  r0, %0;"		   /* &_pcb */								\
		"ldr r1, [r0, #16*4];" /* R1 = saved PSR */						\
		"msr cpsr, r1;"		   /* CPSR = saved PSR */					\
		"ldm r0, {r0-r15};"	   /* Load saved R0-R14,PC, CPSR=SPSR */	\
		: : "r"(_curpcb) : "memory", "cc")


// Save state on exception that may cause a thread switch.
// The exception handler must be declared __naked so it has no implicit
// prologue-epilogue.
// OFFSET is how much LR deviates from the return location:
// 4 for IRQ/FIQ, 8 for Abort/Undef.
#define SaveStateExc(OFFSET)									\
	{  asm volatile(											\
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

// Load state - return from exception
#define LoadStateReturnExc() 											\
	{   asm volatile(													\
			"add sp, sp, #4; "											\
			"ldr r0, =__curpcb;"										\
			"ldr r0, [r0];"												\
			"ldr r1, [r0, #16*4];" /* R1 = saved PSR */					\
			"msr spsr, r1;"		   /* SPSR = saved PSR */				\
			"ldr lr, [r0,#15*4];"  /* LR_irq = saved PC */				\
			"ldm r0, {r0-r14}^; "   /* Load saved user R0-R14 */		\
			"movs pc, lr"			/* Return */						\
			: : : "memory"); }

#else
#error "implement this"
#endif

#endif // __ARM_H__
