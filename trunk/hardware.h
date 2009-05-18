#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "assert.h"

 #include "lpc22xx.h"

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

// Main init function
extern "C" {
void hwinit();
};

// CCLK, PLL, PCLK
enum { CCLK = 14745600 };
enum { PLL_MULT = 4 };
enum { PCLK = CCLK * PLL_MULT };


// Base addresses for peripherals

enum { SPI0_BASE = 0xe0020000,
	   SPI1_BASE = 0xe0030000,
	   SPI_SPCR = 0,
	   SPI_SPSR = 4,
	   SPI_SPDR = 8,
	   SPI_SPCCR = 0xc,
	   SPI_SPINT = 0x1c
};

enum { UART0_BASE = 0xe000c000,
	   UART1_BASE = 0xe0010000,
	   UART_RBR = 0,
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

enum { VIC_BASE = 0xfffff000,
	   VIC_IRQStatus = 0,
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

enum { CS8900A_BASE = 0x82000000,
	   ETH_XD0 = 0,
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


enum { TIMER0_BASE = 0xe0004000,
	   TIMER1_BASE = 0xe0008000,
	   TIMER_IR = 0,
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


enum { GPIO0_BASE = 0xe0028000,
	   GPIO1_BASE = 0xe0028010,
	   GPIO_IOPIN = 0,
	   GPIO_IOSET = 1,
	   GPIO_IODIR = 2,
	   GPIO_IOCLR = 3
};


enum { I2C_BASE = 0xe001c000,
	   I2C_CONSET = 0,
	   I2C_CONCLR = 6,
	   I2C_STAT = 1,
	   I2C_DAT = 2,
	   I2C_ADR = 3,
	   I2C_SCLH = 4,
	   I2C_SCLL = 5
};


#define __irq   __attribute__((interrupt("IRQ"))) __noinstrument
#define __fiq   __attribute__((interrupt("FIQ"))) __noinstrument
#define __abort   __attribute__((interrupt("ABT"))) __noinstrument
#define __undef   __attribute__((interrupt("UNDEF"))) __noinstrument
#define __swi   __attribute__((interrupt("SWI"))) __noinstrument


typedef void (*IRQHandler)();


extern "C" {
void Unexpected_Interrupt() __irq __naked;
void Data_Abort_Exception() __abort __naked;
void Program_Abort_Exception() __abort __naked;
void Undef_Exception() __undef __naked;
void SWI_Trap() __swi __naked;
void busy_wait () __noinstrument __naked;

}

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


// Vectored interrupt controller
class Vic {
	volatile uint32_t* const _base;
	mutable Spinlock _lock;
	uint _num_handlers;
public:
	Vic(uint32_t base) : _base((volatile uint32_t*) base), _num_handlers(0) {
		// Install default IRQ handler
		InstallHandler((uint)-1, Unexpected_Interrupt);
	}
	
	void InstallHandler(uint channel, IRQHandler handler);
	void EnableChannel(uint channel);
	void DisableChannel(uint channel);

	// True if channel has a pending interrupt
	bool ChannelPending(uint channel);

	// Clear pending interrupt status - call prior to return
	void ClearPending();

private:
	static void Unhandled_IRQ() __irq;
};

extern Vic _vic;

#include "timer.h"


void fault0(uint num);
inline void fault(uint num, bool captive = true) { do fault0(num); while (captive); }


extern "C" {
// These symbols are generated by the linker
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss_start;
extern uint8_t _bss_end;
extern uint8_t _etext;
extern uint8_t _stack;
extern uint8_t _estack;
extern uint8_t _xflash;
extern uint8_t _exflash;
extern uint8_t _stack;
extern uint8_t _estack;
};

enum { XRAM_SIZE = 1024*1024 };

#define MALLOC_REGION_START  (&_bss_end)
#define MALLOC_REGION_SIZE   ((&_data + XRAM_SIZE) - &_bss_end)

#define STACK_REGION_START (&_stack)
#define STACK_REGION_SIZE (&_estack - &_stack)

#define DATA_REGION_START (&_data)
#define DATA_REGION_SIZE (&_edata - &_data)

#define TEXT_REGION_START  ((uint8_t*)0)
#define TEXT_REGION_SIZE (&_etext - (uint8_t*)0 + (DATA_REGION_SIZE))

#define XFLASH_REGION_START (&_xflash)
#define XFLASH_REGION_SIZE (&_exflash - &_xflash)

extern void* _main_thread_stack;
extern void* _intr_thread_stack;


#endif // __HARDWARE_H__
