#ifndef __LPC_H__
#define __LPC_H__

#if defined(LPC2294)
#include "lpc2294.h"
#else
#error "Unsupported LPC model"
#endif

#ifdef __GNUC__
#define __irq   __attribute__((interrupt("IRQ"))) __noinstrument
#define __fiq   __attribute__((interrupt("FIQ"))) __noinstrument
#define __abort   __attribute__((interrupt("ABT"))) __noinstrument
#define __undef   __attribute__((interrupt("UNDEF"))) __noinstrument
#define __swi   __attribute__((interrupt("SWI"))) __noinstrument
#else
#error "Unsupported compiler"
#endif

#include "assert.h"

__force_inline uint DisableInterrupts() {
	uint prev;
	asm volatile("mrs r12, cpsr\n"
				 "mov %0, r12\n"
				 "orr r12, #0x80\n"
				 "msr cpsr, r12"
				 : "=r" (prev) : : "r12", "cc", "memory");
	return prev;
}

__force_inline void EnableInterrupts(uint prev) {
	asm volatile("msr cpsr, %0" : : "r" (prev) : "cc", "memory");
}


// Wait for interrupt - enter idle mode
__force_inline void  WaitForInterrupt() {
	PCON = 1;
}


// Test if interrupts are enabled
inline bool IntEnabled() {
	uint32_t cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr) : : );
	return !(cpsr & 0x80);
}


// Test if in user more
inline bool InSystemMode() {
	uint32_t cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr) : : );
	return (cpsr & 0x1f) == 0x1f;
}


class Spinlock {
	mutable uint32_t _cpsr;
	mutable uint _count;

public:
	Spinlock() : _count(0) { }
	~Spinlock() { }
	void Lock() { const uint32_t cpsr = DisableInterrupts(); if (!_count++) _cpsr = cpsr; }
	void Unlock() {
		assert((int)_count > 0);
		if (!--_count) EnableInterrupts(_cpsr);
	}
	void AssertLocked() const { assert(_count); }

	// Abandon lock - must be held.  This resets the lock, which must be held once
	// by the caller.
	void Abandon() { assert(_count == 1);  _count = 0; }

	class Scoped {
		mutable Spinlock& _lock;
	public:
		Scoped(const Spinlock& lock) : _lock((Spinlock&)lock) { _lock.Lock(); }
		~Scoped() { _lock.Unlock(); }
	};
};


#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define BYTE_ORDER LITTLE_ENDIAN


// Peripheral registers.  These are array indices.

enum { SPI_SPCR = 0,
	   SPI_SPSR = 4,
	   SPI_SPDR = 8,
	   SPI_SPCCR = 0xc,
	   SPI_SPINT = 0x1c
};

enum { UART_RBR = 0,
	   UART_THR = 0,
	   UART_IER = 4,
	   UART_IIR = 8,
	   UART_FCR = 8,
	   UART_LCR = 0xc,
	   UART_MCR = 0x10,
	   UART_LSR = 0x14,
	   UART_MSR = 0x18,
	   UART_SCR = 0x1c,
	   UART_DLL = 0,
	   UART_DLM = 4 };

enum { VIC_IRQStatus = 0,
	   VIC_FIQStatus = 1,
	   VIC_RawIntr = 2,
	   VIC_IntSelect = 3,
	   VIC_IntEnable = 4,
	   VIC_IntEnClr = 5,
	   VIC_SoftInt = 6,
	   VIC_SoftIntClear = 7,
	   VIC_Protection = 8,
	   VIC_VectAddr = 12,
	   VIC_DefVectAddr = 13,
	   VIC_VectAddr0 = 64,
	   VIC_VectCntl0 = 128
};

enum { ETH_XD0 = 0,
	   ETH_XD1 = 1,
	   ETH_TxCMD = 2,
	   ETH_TxLength = 3,
	   ETH_ISQ = 4,
	   ETH_PP = 5,
	   ETH_PPDATA0 = 6,
	   ETH_PPDATA1 = 7,

	   // PacketPage offsets
	   ETH_PP_PID = 0,
	   ETH_PP_IOBASE = 0x20,
	   ETH_PP_INTR = 0x22,
	   ETH_PP_DMA_CH = 0x24,
	   ETH_PP_DMA_SOF = 0x26,
	   ETH_PP_DMA_FC = 0x28,
	   ETH_PP_RxDMA_BC = 0x2a,
	   ETH_PP_MemBase = 0x2c,
	   ETH_PP_PROMBase = 0x30,
	   ETH_PP_PROMMask = 0x34,
	   ETH_PP_ISQ = 0x120,
	   ETH_PP_RxCFG = 0x102,
	   ETH_PP_RxEvent = 0x124,
	   ETH_PP_RxCTL = 0x104,
	   ETH_PP_TxEvent = 0x128,
	   ETH_PP_TxCFG = 0x106,
	   ETH_PP_BufCFG = 0x10a,
	   ETH_PP_BusCTL = 0x116,
	   ETH_PP_BufEvent = 0x12c,
	   ETH_PP_RxMISS = 0x130,
	   ETH_PP_TxCOL = 0x132,
	   ETH_PP_LineCTL = 0x112,
	   ETH_PP_LineST = 0x134,
	   ETH_PP_SelfCTL = 0x114,
	   ETH_PP_SelfST = 0x136,
	   ETH_PP_BusST = 0x138,
	   ETH_PP_TestCTL = 0x118,
	   ETH_PP_TDR = 0x13c,
	   ETH_PP_TxCMD = 0x144,
	   ETH_PP_TxLength = 0x146,
	   ETH_PP_LAF = 0x150,
	   ETH_PP_IA = 0x158,

	   // Register numbers
	   ETH_R_ISQ = 0,
	   ETH_R_RxCFG = 3,
	   ETH_R_RxEvent = 4,
	   ETH_R_RxCTL = 5,
	   ETH_R_TxEvent = 8,
	   ETH_R_TxCFG = 7,
	   ETH_R_BufCFG = 0xb,
	   ETH_R_BufEvent = 0xc,
	   ETH_R_RxMISS = 0x10,
	   ETH_R_TxCOL = 0x12,
	   ETH_R_LineCTL = 0x13,
	   ETH_R_LineST = 0x14,
	   ETH_R_SelfCTL = 0x15,
	   ETH_R_SelfST = 0x16,
	   ETH_R_BusST = 0x18,
	   ETH_R_TestCTL = 0x19,
	   ETH_R_TDR = 0x1c
};


enum { TIMER_IR = 0,
	   TIMER_TCR = 1,
	   TIMER_TC = 2,
	   TIMER_PR = 3,
	   TIMER_PC = 4,
	   TIMER_MCR = 5,
	   TIMER_MR0 = 6,
	   TIMER_MR1 = 7,
	   TIMER_MR2 = 8,
	   TIMER_MR3 = 9,
	   TIMER_CCR = 10,
	   TIMER_CR0 = 11,
	   TIMER_CR1 = 12,
	   TIMER_CR2 = 13,
	   TIMER_CR3 = 14,
	   TIMER_EMR = 15
};


enum { GPIO_IOPIN = 0,
	   GPIO_IOSET = 1,
	   GPIO_IODIR = 2,
	   GPIO_IOCLR = 3
};


enum { I2C_CONSET = 0,
	   I2C_CONCLR = 6,
	   I2C_STAT = 1,
	   I2C_DAT = 2,
	   I2C_ADR = 3,
	   I2C_SCLH = 4,
	   I2C_SCLL = 5
};


typedef void (*IRQHandler)();

// Interrupt channels
enum {
	INTCH_WDT = 0,
	INTCH_TIMER0 = 4,
	INTCH_TIMER1 = 5,
	INTCH_UART0 = 6,
	INTCH_UART1 = 7,
	INTCH_PWM0 = 8,
	INTCH_I2C = 9,
	INTCH_SPI0 = 10,
	INTCH_SPI1 = 11,
	INTCH_PLL = 12,
	INTCH_RTC = 13,
	INTCH_EINT0 = 14,
	INTCH_EINT1 = 15,
	INTCH_EINT2 = 16,
	INTCH_EINT3 = 17,
	INTCH_ADC = 18
};

#include "lpc_vic.h"
#include "lpc_timer.h"

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

#else
#error "implement this"
#endif

#endif // __LPC_H__
