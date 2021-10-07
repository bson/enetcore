// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __LPC407X_H__
#define __LPC407X_H__

#define LPC407X

#define CORTEX_M 4

#include "cortex-m4.h"
#include "arm.h"
#include "assert.h"
#include "bits.h"

// Max CCLK = 120MHz
enum { CCLK_MAX = 120000000 };

// On-chip LPC peripheral base addresses
enum { 
    // LPC407x - CAN and USB Host+OTG omitted (they're complex enough
    // to have their own sections with multiple BAs)
    GPIO0_BASE    = 0x20098000,
    GPIO1_BASE    = 0x20098020,
    GPIO2_BASE    = 0x20098040,
    GPIO3_BASE    = 0x20098060,
    GPIO4_BASE    = 0x20098080,
    GPIO5_BASE    = 0x200980a0,
    GPIO0_INTR_BASE = 0x40028084,
    GPIO2_INTR_BASE = 0x400280a4,
    IOCON_BASE    = 0x4002C000,
    GPIO_INT_BASE = 0x40028000,
    EMC_BASE      = 0x2009c000,
    ENET_BASE     = 0x20084000,
    LCD_BASE      = 0x20088000,
    USB_BASE      = 0x2008c000,
    SPIFI_BASE    = 0x20094000,
    SD_BASE       = 0x400c0000,
    UART0_BASE    = 0x4000c000,
    UART1_BASE    = 0x40010000,
    UART2_BASE    = 0x40088000,
    UART3_BASE    = 0x4009c000,
    UART4_BASE    = 0x400a4000,
    TIMER0_BASE   = 0x40004000,
    TIMER1_BASE   = 0x40008000,
    TIMER2_BASE   = 0x40090000,
    TIMER3_BASE   = 0x40094000,
    I2S_BASE      = 0x400a8000,
    I2C0_BASE     = 0x4001c000,
    I2C1_BASE     = 0x4005c000,
    I2C2_BASE     = 0x400a0000,
    SSP0_BASE     = 0x40088000,
    SSP1_BASE     = 0x40030000,
    SSP2_BASE     = 0x400ac000,
    PCON_BASE     = 0x400fc0c0,
    PLL0_BASE     = 0x400fc080,
    PLL1_BASE     = 0x400fc0a0,
    ADC_BASE      = 0x40034000,
    RTC_BASE      = 0x40024000,
    RTC_EV_BASE   = 0x40024000,
    QEI_BASE      = 0x400bc000,
    PWM0_BASE     = 0x40014000,
    PWM1_BASE     = 0x40018000,
    MCPWM_BASE    = 0x400b8000,
    WWDT_BASE     = 0x40000000,
    DAC_BASE      = 0x4008c000,
    CMP_BASE      = 0x40020000,
    GPDMA_BASE    = 0x20080000,
    CRC_BASE      = 0x20090000,
    EEPROM_BASE   = 0x00200000, // Yes, that's the actual address... !!!
};


// Number of peripherals and other limits
enum {
    INT_NUM   = 41,             // 41 IRQs (plus 16 fault handlers)
    EINTR_NUM = 4,
    PWM_NUM   = 2,
    GPIO_NUM  = 5,
    UART_NUM  = 4,
    SPI_NUM   = 3,
    I2C_NUM   = 3,
    TIMER_NUM = 4,
    CMP_NUM   = 2,
    DMA_CH_NUM= 8,

    // 4032 bytes of EEPROM; 63 pages of 64 bytes
    EEPROM_PAGES = 63,
    EEPROM_SIZE  = EEPROM_PAGES*64
};

// Interrupt IRQs
enum {
    WDT_IRQ      = 0,
    TIMER0_IRQ   = 1,
    TIMER1_IRQ   = 2,
    TIMER2_IRQ   = 3,
    TIMER3_IRQ   = 4,
    UART0_IRQ    = 5,
    UART1_IRQ    = 6,
    UART2_IRQ    = 7,
    UART3_IRQ    = 8,
    PWM1_IRQ     = 9,
    I2C0_IRQ     = 10,
    I2C1_IRQ     = 11,
    I2C2_IRQ     = 12,

    SSP0_IRQ     = 14,
    SSP1_IRQ     = 15,
    PLL0_IRQ     = 16,
    RTC_IRQ      = 17,
    EINT0_IRQ    = 18,
    EINT1_IRQ    = 19,
    EINT2_IRQ    = 20,
    EINT3_IRQ    = 21,
    ADC_IRQ      = 22,
    BOD_IRQ      = 23,
    USB_IRQ      = 24,
    CAN_IRQ      = 25,
    DMA_IRQ      = 26,
    I2S_IRQ      = 27,
    ENET_IRQ     = 28,
    SD_IRQ       = 29,
    MCPWM_IRQ    = 30,
    QEI_IRQ      = 31,
    PLL1_IRQ     = 32,
    USBACT_IRQ   = 33,
    CANACT_IRQ   = 34,
    UART4_IRQ    = 35,
    SSP2_IRQ     = 36,
    LCD_IRQ      = 37,
    GPIO_IRQ     = 38,
    PWM0_IRQ     = 39,
    EEPROM_IRQ   = 40
};

extern "C" {

// Power control
DEV_CTL_REG(PCONP);
DEV_CTL_REG(PCONP1);
DEV_CTL_REG(PBOOST);

// Clock selection
DEV_CTL_REG(EMCCLKSEL);
DEV_CTL_REG(CCLKSEL);
DEV_CTL_REG(USBCLKSEL);
DEV_CTL_REG(CLKSRCSEL);
DEV_CTL_REG(PCLKSEL);
DEV_CTL_REG(SPIFICLKSEL);

// External interrupts
DEV_CTL_REG(EXTINT);
DEV_CTL_REG(EXTMODE);
DEV_CTL_REG(EXTPOLAR);

// Device and peripheral reset
DEV_CTL_REG(RSID);
DEV_CTL_REG(RSTCON0);
DEV_CTL_REG(RSTCON1);

// External memory control
DEV_CTL_REG(EMCDLYCTL);
DEV_CTL_REG(EMCCAL);

// Miscellaneous system control registers
DEV_CTL_REG(SCS);
DEV_CTL_REG(IRCCTRL);
DEV_CTL_REG(LCD);
DEV_CTL_REG(CANSLEEPCLR);
DEV_CTL_REG(CANWAKEFLAGS);
DEV_CTL_REG(USBINTST);
DEV_CTL_REG(CLKOUTCFG);
DEV_CTL_REG(MATRIXARB);
DEV_CTL_REG(MEMMAP);

// Flash accelerator
DEV_CTL_REG(FLASHCFG);

};

// Wait for interrupt - enter idle mode
extern void WaitForInterrupt();

// PCONP bits
enum {
    // PCONP
    PCLCD = BIT0,
    PCTIM0 = BIT1,
    PCTIM1 = BIT2,
    PCUART0 = BIT3,
    PCUART1 = BIT4,
    PCPWM0 = BIT5,
    PCPWM1 = BIT6,
    PCI2C0 = BIT7,
    PCUART4 = BIT8,
    PCRTC = BIT9,
    PCSSP1 = BIT10,
    PCEMC = BIT11,
    PCADC = BIT12,
    PCCAN1 = BIT13,
    PCCAN2 = BIT14,
    PCGPIO = BIT15,
    PCSPIFI = BIT16,
    PCMCPWM = BIT17,
    PCQEI = BIT18,
    PCI2C1 = BIT19,
    PCSSP2 = BIT20,
    PCSSP0 = BIT21,
    PCTIM2 = BIT22,
    PCTIM3 = BIT23,
    PCUART2 = BIT24,
    PCUART3 = BIT25,
    PCI2C2 = BIT26,
    PCI2S = BIT27,
    PCSDC = BIT28,
    PCGPDMA = BIT29,
    PCENET = BIT30,
    PCUSB = BIT31,

    // PCONP1
    PCCMP = BIT3
};

// To help build pin initialization tables

// D type.  See table 84.
#define D_IOCON_GPIO(MODE, HYS, INV, SLEW, OD) \
    (((MODE) << 3) | ((HYS) << 5) | ((INV) << 6) | ((SLEW) << 9) | ((OD) << 10))

#define D_IOCON_FUNC(FUNC) \
    (FUNC)

// Note: FILTER=1 disables filter
#define A_IOCON(FUNC, MODE, INV, ADMODE, FILTER, OD, DACEN) \
    ((FUNC) | ((MODE) << 3) | ((INV) << 6) | ((ADMODE) << 7) | ((FILTER) << 8) | ((OD) << 10) | ((DACEN) << 16))

// A type.  See table 86.
#define A_IOCON_GPIO(MODE, INV, FILTER, OD) \
    A_IOCON(0, (MODE), (INV), 1, (FILTER), (OD), 0)

#define A_IOCON_ADC(FUNC) \
    A_IOCON((FUNC), 0, 0, 0, 1, 0, 0)

#define A_IOCON_DAC(FUNC) \
    A_IOCON((FUNC), 0, 0, 1, 1, 0, 1)

#define A_IOCON_FUNC(FUNC) \
    A_IOCON((FUNC), 0, 0, 1, 1, 0, 0)

// U type.  See table 88.
#define U_IOCON_FUNC(FUNC) \
    (FUNC)

// I type.  See table 90.
#define I_IOCON(FUNC, INV, HS, HIDRIVE) \
    ((FUNC) | ((INV) << 6) | ((HS) << 8) | ((HIDRIVE) << 9))

#define I_IOCON_GPIO(INV, HS, HIDRIVE) \
    I_IOCON(0, (INV), (HS), (HIDRIVE))

// W type. See table 92.
#define W_IOCON(FUNC, MODE, HYS, INV, ADMODE, FILTER, SLEW, OD) \
    ((FUNC) | ((MODE) << 3) | ((HYS) << 5) || ((INV) << 6) | ((ADMODE) << 7) | ((FILTER) << 8) \
     | ((SLEW) << 9) | ((OD) << 10))

#define W_IOCON_GPIO(MODE, HYS, INV, FILTER, SLEW, OD) \
    W_IOCON(0, (MODE), (HYS), (INV), 1, (FILTER), (SLEW), (OD))

#define W_IOCON_CMP(FILTER) \
    W_IOCON(0b101, IOCON_MODE_NONE, 0, 0, 1, 1, 0, 0)

#define W_IOCON_FUNC(FUNC) \
    W_IOCON((FUNC), IOCON_MODE_NONE, 0, 0, 1, 1, 0, 0)


enum { 
    IOCON_MODE_NONE = 0,   // No pull-up or pull-down
    IOCON_MODE_PULLDOWN = 1, // Pull-down
    IOCON_MODE_PULLUP = 2,
    IOCON_MODE_REPEATER = 3 // Pull follows output
};
    
extern "C" {
extern volatile uint32_t IOCON_P0[32];
extern volatile uint32_t IOCON_P1[32];
extern volatile uint32_t IOCON_P2[32];
extern volatile uint32_t IOCON_P3[32];
extern volatile uint32_t IOCON_P4[32];
extern volatile uint32_t IOCON_P5[4];
};

#endif // __LPC407X_H__
