#ifndef __LPC2294_H__
#define __LPC2294_H__


// On-chip LPC peripheral base addresses
enum { SPI0_BASE	= 0xe0020000,
	   SPI1_BASE	= 0xe0030000,
	   UART0_BASE	= 0xe000c000,
	   UART1_BASE	= 0xe0010000,
	   VIC_BASE		= 0xfffff000,
	   CS8900A_BASE	= 0x82000000,
	   TIMER0_BASE	= 0xe0004000,
	   TIMER1_BASE	= 0xe0008000,
	   GPIO0_BASE	= 0xe0028000,
	   GPIO1_BASE	= 0xe0028010,
	   I2C_BASE		= 0xe001c000
};


// Number of peripherals
#define GPIO_NUM 2
#define SPI_NUM 2
#define UART_NUM 2
#define TIMER_NUM 2
#define EINTR_NUM 4


// Other peripherals that don't have clean encapsulations yet

// Memory Accelerator Module (MAM)
#define MAMCR (*((volatile unsigned char*)0xe01fc000))
#define MAMTIM (*((volatile unsigned char*)0xe01fc004))

// Memory map control
#define MEMMAP (*((volatile unsigned char*)0xe01fc040))

// Phase Locked Loop (PLL)
#define PLLCON (*((volatile unsigned char*)0xe01fc080))
#define PLLCFG (*((volatile unsigned char*)0xe01fc084))
#define PLLSTAT (*((volatile unsigned short*)0xe01fc088))
#define PLLFEED (*((volatile unsigned char*)0xe01fc08c))

// VPB Divider
#define VPBDIV (*((volatile unsigned char*)0xe01fc100))

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


// EMC
#define BCFG0 (*((volatile unsigned long*)0xffe00000))
#define BCFG1 (*((volatile unsigned long*)0xffe00004))
#define BCFG2 (*((volatile unsigned long*)0xffe00008))
#define BCFG3 (*((volatile unsigned long*)0xffe0000c))

// ADC
#define ADCR (*((volatile unsigned long*)0xe0034000))
#define ADDR (*((volatile unsigned long*)0xe0034004))

// Pin Connect Block
#define PINSEL0 (*((volatile unsigned long *)0xe002c000))
#define PINSEL1 (*((volatile unsigned long *)0xe002c004))
#define	PINSEL2 (*((volatile unsigned long *)0xe002c014))

#endif // __LPC2294_H__
