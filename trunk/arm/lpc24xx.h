#ifndef __LPC24XX_H__
#define __LPC24XX_H__

#define LPC24XX
#define ENHANCED_GPIO
#define ENHANCED_VIC

// Max CCLK = 75MHz
enum { CCLK_MAX = 75000000 };

// On-chip LPC peripheral base addresses
enum { SPI0_BASE	= 0xe0068000,
	   SPI1_BASE	= 0xe0030000,

	   UART0_BASE	= 0xe000c000,
	   UART1_BASE	= 0xe0010000,
	   UART2_BASE	= 0xe0078000,
	   UART3_BASE	= 0xe007c000,

	   GPIO0_BASE	= 0xe0028000, // Legacy APB
	   GPIO1_BASE	= 0xe0028010, // Legacy APB

	   GPIO0E_BASE	= 0x3fffc000, // Local bus accessible (enhanced)
	   GPIO1E_BASE	= 0x3fffc020,
	   GPIO2E_BASE	= 0x3fffc040,
	   GPIO3E_BASE	= 0x3fffc060,
	   GPIO4E_BASE	= 0x3fffc080,

       // GPIO ports 0 and 2 can generate interrupts
       GPIO0E_INT_BASE = 0xe0028084,
       GPIO2E_INT_BASE = 0xe00280a4,
       GPIOE_IntStatus = 0xe0028080,

	   VIC_BASE		= 0xfffff000,
	   CS8900A_BASE	= 0x82000000,

	   TIMER0_BASE	= 0xe0004000,
	   TIMER1_BASE	= 0xe0008000,
	   TIMER2_BASE	= 0xe0070000,
	   TIMER3_BASE	= 0xe0074000,

	   I2C0_BASE	= 0xe001c000,
	   I2C1_BASE	= 0xe005c000,
	   I2C2_BASE	= 0xe008c000,

	   PLL_BASE		= 0xe01fc080,

       ETH_BASE = 0xffe00000
};


// Number of peripherals
#define GPIO_NUM 2              // APB
#define GPIOE_NUM 5             // 24xx enhanced GPIO
#define SPI_NUM 2
#define UART_NUM 4
#define TIMER_NUM 4
#define EINTR_NUM 4


// Other peripherals that don't have clean encapsulations yet

// Memory Accelerator Module (MAM)
#define MAMCR (*((volatile unsigned char*)0xe01fc000))
#define MAMTIM (*((volatile unsigned char*)0xe01fc004))

// Memory map control
#define MEMMAP (*((volatile unsigned char*)0xe01fc040))

// Power Control
#define PCON (*((volatile unsigned char*)0xe01fc0c0))
#define PCONP (*((volatile unsigned long*)0xe01fc0c4))

// External interrupts
#define EXTINT (*((volatile unsigned char*)0xe01fc140))
#define EXTWAKE (*((volatile unsigned char*)0xe01fc144))
#define EXTMODE (*((volatile unsigned char*)0xe01fc148))
#define EXTPOLAR (*((volatile unsigned char*)0xe01fc14c))

// Watchdog
#define WDMOD (*((volatile unsigned char*)0xe0000000))
#define WDTC (*((volatile unsigned long*)0xe0000004))
#define WDFEED (*((volatile unsigned char*)0xe0000008))
#define WDTV (*((volatile unsigned long*)0xe000000c))

// ADC
#define AD0CR (*((volatile unsigned long*)0xe0034000))
#define AD0GDR (*((volatile unsigned long*)0xe0034004))
#define AD0STAT (*((volatile unsigned long*)0xe0034030))
#define AD0INTEN (*((volatile unsigned long*)0xe003400c))
#define AD0DR ((volatile unsigned long*)0xe003400c)   // AD0DR[n]

// Pin Connect Block
#define PINSEL ((volatile unsigned long*)0xe002c000) // PINSEL[n]  | n <= 11
#define PIMODE ((volatile unsigned long*)0xe002c040) // PINMODE[n] | n <= 9


// Generic ARM support
#include "arm.h"
#include "assert.h"
#include "spinlock.h"

// Wait for interrupt - enter idle mode
void  WaitForInterrupt() __finline;
inline void  WaitForInterrupt() {
	PCON = 1;
}


// Peripheral registers.  These are array indices.  Some peripherals
// use byte arrays, others words.

enum { SPI_SPCR0 = 0,
	   SPI_SPCR1 = 4,
	   SPI_SPDR = 8,
	   SPI_SPSR = 0xc,
       SPI_CPSR = 0x10,
       SPI_IMSC = 0x14,
       SPI_RIS = 0x18,
	   SPI_MIS = 0x1c,
       SPI_ICR = 0x20,
       SPI_DMACR = 0x24
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
       UART_ACR = 0x20,
       UART_ICR = 0x24,
       UART_FDR = 0x28,
       UART_TER = 0x30,
	   UART_DLL = 0,
	   UART_DLM = 4
};


enum { VIC_IRQStatus = 0,
	   VIC_FIQStatus = 0x004/4,
	   VIC_RawIntr = 0x008/4,
	   VIC_IntSelect = 0x00c/4,
	   VIC_IntEnable = 0x010/4,
	   VIC_IntEnClr = 0x014/4,
	   VIC_SoftInt = 0x018/4,
	   VIC_SoftIntClear = 0x01c/4,
	   VIC_Protection = 0x020/4,
       VIC_SWPriorityMask = 0x024/4,
	   VIC_VectAddr0 = 0x100/4,
       VIC_VectPriority0 = 0x200/4,
	   VIC_Address = 0xf00/4,
};


enum { TIMER_IR = 0x00/4,
	   TIMER_TCR = 0x04/4,
	   TIMER_TC = 0x08/4,
	   TIMER_PR = 0x0c/4,
	   TIMER_PC = 0x10/4,
	   TIMER_MCR = 0x14/4,
	   TIMER_MR0 = 0x18/4,
	   TIMER_MR1 = 0x1c/4,
	   TIMER_MR2 = 0x20/4,
	   TIMER_MR3 = 0x24/4,
	   TIMER_CCR = 0x28/4,
	   TIMER_CR0 = 0x2c/4,
	   TIMER_CR1 = 0x30/4,
	   TIMER_EMR = 0x3c/4,
       TIMER_CTCR = 0x70/4
};


enum { GPIO_IOPIN = 0,
	   GPIO_IOSET = 1,
	   GPIO_IODIR = 2,
	   GPIO_IOCLR = 3
};


enum { GPIOE_FIODIR = 0,        // Indices to 32-bit register array
       GPIOE_FIOMASK = 0x10/4,
       GPIOE_FIOPIN = 0x14/4,
       GPIOE_FIOSET = 0x18/4,
       GPIOE_FIOCLR = 0x1c/4
};


// Offset is GPIOE_IntStatus + (port/2*0x20) + register*4
// LPC24xx User Manual chapter 10
enum { GPIOE_IntEnR = 0x10/4,        // Indices to 32-bit register array
       GPIOE_IntEnF = 0x14/4,
       GPIOE_IntStatR = 0x4/4,
       GPIOE_IntStatF = 0x8/4,
       GPIOE_IntClr = 0xc/4
};


enum { I2C_CONSET = 0x00/4,
	   I2C_STAT = 0x04/4,
	   I2C_DAT = 0x08/4,
	   I2C_ADR = 0x0c/4,
	   I2C_SCLH = 0x10/4,
	   I2C_SCLL = 0x14/4,
	   I2C_CONCLR = 0x18/4
};


// Ethernet registers
enum {
    ETH_MAC1 = 0x000/4,
    ETH_MAC2 = 0x004/4,
    ETH_IPGT = 0x008/4,
    ETH_IPGR = 0x00c/4,
    ETH_CLRT = 0x010/4,
    ETH_MAXF = 0x014/4,
    ETH_SUPP = 0x018/4,
    ETH_TEST = 0x01c/4,
    ETH_MCFG = 0x020/4,
    ETH_MCMD = 0x024/4,
    ETH_MADR = 0x028/4,
    ETH_MWTD = 0x02c/4,
    ETH_MRDD = 0x030/4,
    ETH_MIND = 0x034/4,
    ETH_SA0 = 0x040/4,
    ETH_SA1 = 0x044/4,
    ETH_SA2 = 0x048/4,
    ETH_Command = 0x100/4,
    ETH_Status = 0x104/4,
    ETH_RxDescriptor = 0x108/4,
    ETH_RxStatus = 0x10c/4,
    ETH_RxDescriptorNumber = 0x110/4,
    ETH_RxProduceIndex = 0x114/4,
    ETH_RxConsumeIndex = 0x118/4,
    ETH_TxDescriptor = 0x11c/4,
    ETH_TxStatus = 0x120/4,
    ETH_TxDescriptorNumber = 0x124/4,
    ETH_TxProduceIndex = 0x128/4,
    ETH_TxConsumeIndex = 0x12c/4,
    ETH_TSV0 = 0x158/4,
    ETH_TSV1 = 0x15c/4,
    ETH_RSV = 0x160/4,
    ETH_FlowControlCounter = 0x170/4,
    ETH_FlowControlStatus = 0x174/4,
    ETH_RxFilterCtrl = 0x200/4,
    ETH_RxFilterWoLStatus = 0x204/4,
    ETH_RxFilterWoLClear = 0x208/4,
    ETH_HashFilterL = 0x210/4,
    ETH_HashFilterH = 0x214/4,
    ETH_IntStatus = 0xfe0/4,
    ETH_IntEnable = 0xfe4/4,
    ETH_IntClear = 0xfe8/4,
    ETH_IntSet = 0xfec/4,
    ETH_PowerDown = 0xff4/4
};


// Interrupt channels
enum {
	INTCH_WDT = 0,
    INTCH_DbgCommRx = 1,        // ICE use
    INTCH_DbgCommTx = 2,        // ICE use
	INTCH_TIMER0 = 4,
	INTCH_TIMER1 = 5,
	INTCH_UART0 = 6,
	INTCH_UART1 = 7,
	INTCH_PWM0 = 8,
	INTCH_I2C0 = 9,
	INTCH_SPI0 = 10,
	INTCH_SPI1 = 11,
	INTCH_PLL = 12,
	INTCH_RTC = 13,
	INTCH_EINT0 = 14,
	INTCH_EINT1 = 15,
	INTCH_EINT2 = 16,
	INTCH_EINT3 = 17,
	INTCH_ADC = 18,
    INTCH_I2C1 = 19,
    INTCH_BOD = 20,
    INTCH_ETH = 21,
    INTCH_USB = 22,
    INTCH_CAN = 23,
    INTCH_SDMMC = 24,
    INTCH_GPDMA = 25,
    INTCH_TIMER2 = 26,
    INTCH_TIMER3 = 27,
    INTCH_UART2 = 28,
    INTCH_UART3 = 29,
    INTCH_I2C2 = 30,
    INTCH_I2S = 31
};


#include "lpc_vic.h"
#include "lpc_timer.h"

#endif // __LPC22XX_H__
