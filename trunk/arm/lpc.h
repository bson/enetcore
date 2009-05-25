#ifndef __LPC_H__
#define __LPC_H__

#if LPC == 2294
#include "lpc2294.h"
#else
#error "Unsupported LPC model"
#endif

#include "assert.h"

// Generic ARM support
#include "arm.h"

// Wait for interrupt - enter idle mode
__force_inline void  WaitForInterrupt() {
	PCON = 1;
}


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

#endif // __LPC_H__
