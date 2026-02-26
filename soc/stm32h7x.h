// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32H7X_H__
#define __STM32H7X_H__

#define STM32H7X

#include <stdint.h>

#define CORTEX_M 7

#define HAVE_HW_CRC

#include "arch/armv7m/cortex-m7.h"
#include "arch/armv7m/arm.h"
#include "core/assert.h"
#include "core/bits.h"

#define _h7x_addrof(SYM) ((uintptr_t)&(SYM))

extern "C" {
extern int ADDR_ITCM;
extern int ADDR_FLASHB1;
extern int ADDR_FLASHB2;
extern int ADDR_DTCM;
extern int ADDR_SRAM1;
extern int ADDR_SRAM2;
extern int ADDR_SRAM3;
extern int ADDR_SRAM4;
extern int ADDR_BKPSRAM;
extern int ADDR_SRAM;
extern int ADDR_RAM;
extern int ADDR_FMCB1;
extern int ADDR_FMCB2;
extern int ADDR_FMCB3;
extern int ADDR_FMCB4;
extern int ADDR_FMCB5;
extern int ADDR_FMCB6;
}

#define BASE_ITCM     (_h7x_addrof(ADDR_ITCM))
#define BASE_FLASHB1  (_h7x_addrof(ADDR_FLASHB1))
#define BASE_FLASHB2  (_h7x_addrof(ADDR_FLASHB2))

/* RAM */
#define BASE_DTCM     (_h7x_addrof(ADDR_DTCM))
#define BASE_SRAM1    (_h7x_addrof(ADDR_SRAM1))
#define BASE_SRAM2    (_h7x_addrof(ADDR_SRAM2))
#define BASE_SRAM3    (_h7x_addrof(ADDR_SRAM3))
#define BASE_SRAM4    (_h7x_addrof(ADDR_SRAM4))
#define BASE_BKPSRAM  (_h7x_addrof(ADDR_BKPSRAM))
#define BASE_AXI_SRAM (_h7x_addrof(ADDR_AXI_SRAM))
#define BASE_QSPI_RAM (_h7x_addrof(ADDR_QSPI_RAM))

/* FMC banks */
#define BASE_FMCB1    (_h7x_addrof(ADDR_FMCB1))
#define BASE_FMCB2    (_h7x_addrof(ADDR_FMCB2))
#define BASE_FMCB3    (_h7x_addrof(ADDR_FMCB3))
#define BASE_FMCB4    (_h7x_addrof(ADDR_FMCB4))
#define BASE_FMCB5    (_h7x_addrof(ADDR_FMCB5))
#define BASE_FMCB6    (_h7x_addrof(ADDR_FMCB6))

// Max CPU CK = 480MHz
// Max AHB CK = 240MHz
enum {
    CPU_FREQ_MAX = 480000000,
    AHB_FREQ_MAX = 240000000,
};

enum { 
    // APB1

    BASE_TIM2         = 0x40000000,
    BASE_TIM3         = 0x40000400,
    BASE_TIM4         = 0x40000800,
    BASE_TIM5         = 0x40000C00,
    BASE_TIM6         = 0x40001000,
    BASE_TIM7         = 0x40001400,
    BASE_TIM12        = 0x40001800,
    BASE_TIM13        = 0x40001C00,
    BASE_TIM14        = 0x40002000,
    BASE_LPTIM1       = 0x40002400,
    BASE_SPI2_I2S2    = 0x40003800, // SPI/I2S
    BASE_SPI3_I2S3    = 0x40003C00, // SPI/I2S
    BASE_SPDIFRX1     = 0x40004000,
    BASE_USART2       = 0x40004400,
    BASE_USART3       = 0x40004800,
    BASE_UART4        = 0x40004C00,
    BASE_UART5        = 0x40005000,
    BASE_I2C1         = 0x40005400,
    BASE_I2C2         = 0x40005800,
    BASE_I2C3         = 0x40005C00,
    BASE_HDMI_CEC     = 0x40006C00,
    BASE_DAC1         = 0x40007400,
    BASE_UART7        = 0x40007800,
    BASE_UART8        = 0x40007C00,
    BASE_CRS          = 0x40008400,
    BASE_SWPMI        = 0x40008800,
    BASE_OPAMP        = 0x40009000,
    BASE_MDIOS        = 0x40009400,

    // APB2

    BASE_TIM1         = 0x40010000,
    BASE_TIM8         = 0x40010400,
    BASE_USART1       = 0x40011000,
    BASE_USART6       = 0x40011400,
    BASE_SPI1         = 0x40013000,
    BASE_SPI4         = 0x40013400,
    BASE_TIM15        = 0x40014000,
    BASE_TIM16        = 0x40014400,
    BASE_TIM17        = 0x40014800,
    BASE_SPI5         = 0x40015000,
    BASE_SAI1         = 0x40015800,
    BASE_SAI2         = 0x40015C00,
    BASE_SAI3         = 0x40016000,
    BASE_DFSDM1       = 0x40017000,
    BASE_HRTIM        = 0x40017400,

    // AHB1

    BASE_DMA1         = 0x40020000,
    BASE_DMA2         = 0x40020400,
    BASE_DMAMUX1      = 0x40020800,
    BASE_ADCCOM       = 0x40022000,
    BASE_ETH          = 0x40028000,
    BASE_USB1_OTG_HS  = 0x40040000,
    BASE_USB2_OTG_FS  = 0x40080000,
    BASE_FDCAN1       = 0x4000A000,
    BASE_FDCAN2       = 0x4000A400,
    BASE_CAN_CCU      = 0x4000A800,
    BASE_CAN_MSGRAM   = 0x4000AC00,

    // AHB2

    BASE_DCMI         = 0x48020000,
    BASE_CRYP         = 0x48021000,
    BASE_HASH         = 0x48021400,
    BASE_RNG          = 0x48021800,
    BASE_SDMMC2       = 0x48022400,
    BASE_SDMMC2_DELAY = 0x48022800,
    BASE_RAMECC2      = 0x48023000,

    // APB3

    BASE_LTDC         = 0x50001000,
    BASE_WWDG         = 0x50003000,

    // AHB3

    BASE_GPV          = 0x51000000, // AXI interconnect matrix (AXIM)
    BASE_MDMA         = 0x52000000,
    BASE_DMA2D        = 0x52001000,
    BASE_FLASH        = 0x52002000, // Control regs
    BASE_JPEG         = 0x52003000,
    BASE_FMC          = 0x52004000,
    BASE_QUADSPI      = 0x52005000,
    BASE_SDMMC1       = 0x52007000,
    BASE_SDMMC1_DELAY = 0x52008000,
    BASE_RAMECC1      = 0x52009000,

    // APB4

    BASE_EXTI         = 0x58000000,
    BASE_SYSCFG       = 0x58000400,
    BASE_LPUART1      = 0x58000C00,
    BASE_SPI6         = 0x58001400,
    BASE_I2C4         = 0x58001C00,
    BASE_LPTIM2       = 0x58002400,
    BASE_LPTIM3       = 0x58002800,
    BASE_LPTIM4       = 0x58002C00,
    BASE_LPTIM5       = 0x58003000,
    BASE_VREFBUF      = 0x58003C00,
    BASE_COMP         = 0x58003800, // COMP1-COMP2
    BASE_RTC          = 0x50004000,
    BASE_IWDG         = 0x58004800,
    BASE_SAI4         = 0x58005400,

    // AHB4

    BASE_GPIOA        = 0x58020000,
    BASE_GPIOB        = 0x58020400,
    BASE_GPIOC        = 0x58020800,
    BASE_GPIOD        = 0x58020c00,
    BASE_GPIOE        = 0x58021000,
    BASE_GPIOF        = 0x58021400,
    BASE_GPIOG        = 0x58021800,
    BASE_GPIOH        = 0x58021c00,
    BASE_GPIOI        = 0x58022000,
    BASE_GPIOJ        = 0x58022400,
    BASE_GPIOK        = 0x58022800,
    BASE_CRC          = 0x58024C00,
    BASE_PWR          = 0x58024800,
    BASE_RCC          = 0x58024400,
    BASE_BDMA         = 0x58025400,
    BASE_DMAMUX2      = 0x58025800,
    BASE_ADC3         = 0x58026000,
    BASE_HSEM         = 0x58026400,
    BASE_RAMECC3      = 0x58027000,

    BASE_DBGMCU       = 0x5C001000,

    BASE_FMC_MEM      = 0x60000000 // FMC address space
};


// Number of peripherals and other limits
enum {
    INT_NUM   = 81,             // 81 IRQs (plus 16 fault vectors (some unused))
    EINTR_NUM = 16,
    PWM_NUM   = 2,
    GPIO_NUM  = 5,  // A-E
    UART_NUM  = 8,
    SPI_NUM   = 6,
    I2C_NUM   = 3,
    TIMER_NUM = 12,
    LPTIMER_NUM = 5,
    BKPSRAM_SIZE = 4*1024
};

// Interrupt IRQs
enum {
    INTR_WWDG1          = 0,
    INTR_PVD            = 1,
    INTR_TAMP_STAMP     = 2,
    INTR_RTC_WKUP       = 3,
    INTR_FLASH          = 4,
    INTR_RCC            = 5,
    INTR_EXTI0          = 6,
    INTR_EXTI1          = 7,
    INTR_EXTI2          = 8,
    INTR_EXTI3          = 9,
    INTR_EXTI4          = 10,
    INTR_DMA1_Stream0   = 11,
    INTR_DMA1_Stream1   = 12,
    INTR_DMA1_Stream2   = 13,
    INTR_DMA1_Stream3   = 14,
    INTR_DMA1_Stream4   = 15,
    INTR_DMA1_Stream5   = 16,
    INTR_DMA1_Stream6   = 17,
    INTR_ADC1_2         = 18,
    INTR_CAN1_TX        = 19,
    INTR_CAN1_RX0       = 20,
    INTR_CAN1_RX1       = 21,
    INTR_CAN1_SCE       = 22,
    INTR_EXTI5_9        = 23,
    INTR_TIM1_BRK       = 24,
    INTR_TIM1_UP        = 25,
    INTR_TIM1_TRG_COM   = 26,
    INTR_TIM1_CC        = 27,
    INTR_TIM2           = 28,
    INTR_TIM3           = 29,
    INTR_TIM4           = 30,
    INTR_I2C1_EV        = 31,
    INTR_I2C1_ER        = 32,
    INTR_I2C2_EV        = 33,
    INTR_I2C2_ER        = 34,
    INTR_SPI1           = 35,
    INTR_SPI2           = 36,
    INTR_USART1         = 37,
    INTR_USART2         = 38,
    INTR_USART3         = 39,
    INTR_EXTI10_15      = 40,
    INTR_RTC_Alarm      = 41,
    // 42 unused
    INTR_TIM8_BRK_TIM12 = 43,
    INTR_TIM8_UP_TIM13  = 44,
    INTR_TIM8_TRG_COM_TIM14 = 45,
    INTR_TIM8_CC        = 46,
    INTR_DMA1_Stream7   = 47,
    INTR_FSMC           = 48,
    INTR_SDMMC1         = 49,
    INTR_TIM5           = 50,
    INTR_SPI3           = 51,
    INTR_UART4          = 52,
    INTR_UART5          = 53,
    INTR_TIM6_DAC       = 54,
    INTR_TIM7           = 55,
    INTR_DMA2_Stream0   = 56,
    INTR_DMA2_Stream1   = 57,
    INTR_DMA2_Stream2   = 58,
    INTR_DMA2_Stream3   = 59,
    INTR_DMA2_Stream4   = 60,
    INTR_ETH            = 61,
    INTR_ETH_WKUP       = 62,
    INTR_FDCAN_CAL      = 63,
    INTR_CM7_SEV_IT     = 64,      // Cortex-M7 Send even interrupt
    // 65-67 unused
    INTR_DMA2_Stream5   = 68,
    INTR_DMA2_Stream6   = 69,
    INTR_DMA2_Stream7   = 70,
    INTR_USART6         = 71,
    INTR_I2C3_EV        = 72,
    INTR_I2C3_ER        = 73,
    INTR_OTG_HS_EP1_OUT = 74,
    INTR_OTG_HS_EP1_IN  = 75,
    INTR_OTG_HS_WKUP    = 76,
    INTR_OTG_HS         = 77,
    INTR_DCMI           = 78,
    INTR_CRYP           = 79,
    INTR_HASH_RNG       = 80,
    INTR_FPU            = 81,
    INTR_UART7          = 82,
    INTR_UART8          = 83,
    INTR_SPI4           = 84,
    INTR_SPI5           = 85,
    INTR_SPI6           = 86,
    INTR_SAI1           = 87,
    INTR_LTDC           = 88,
    INTR_LTDC_ER        = 89,
    INTR_DMA2D          = 90,
    INTR_SAI2           = 91,
    INTR_QSPI           = 92,
    INTR_LPTIM1         = 93,
    INTR_CEC            = 94,
    INTR_I2C4_EV        = 95,
    INTR_I2C4_ER        = 96,
    INTR_SPDIF          = 97,
    INTR_OTG_FS_EP1_OUT = 98,
    INTR_OTG_FS_EP1_IN  = 99,
    INTR_OTG_FS_WKUP    = 100,
    INTR_OTG_FS         = 101,
    INTR_DMAMUX1_OV     = 102,
    INTR_HRTIM1_MST     = 103,

    INTR_HRTIM1_TIMA    = 104,
    INTR_HRTIM_TIMB     = 105,
    INTR_HRTIM1_TIMC    = 106,
    INTR_HRTIM1_TIMD    = 107,
    INTR_HRTIM_TIME     = 108,
    INTR_HRTIM1_FLT     = 109,
    INTR_DFSDM1_FLT0    = 110,
    INTR_DFSDM1_FLT1    = 111,
    INTR_DFSDM1_FLT2    = 112,
    INTR_DFSDM1_FLT3    = 113,
    INTR_SAI3           = 114,
    INTR_SWPMI1         = 115,
    INTR_TIM15          = 116,
    INTR_TIM16          = 117,
    INTR_TIM17          = 118,
    INTR_MDIOS_WKUP     = 119,
    INTR_MDIOS          = 120,
    INTR_JPEG           = 121,
    INTR_MDMA           = 122,
    // 123 unused
    INTR_SDMMC2         = 124,
    INTR_HSEM0          = 125,
    // 126 unused
    INTR_ADC3           = 127,
    INTR_DMAMUX2_OVR    = 128,
    INTR_BDMA_CH0       = 129,
    INTR_BDMA_CH1       = 130,
    INTR_BDMA_CH2       = 131,
    INTR_BDMA_CH3       = 132,
    INTR_BDMA_CH4       = 133,
    INTR_BDMA_CH5       = 134,
    INTR_BDMA_CH6       = 135,
    INTR_BDMA_CH7       = 136,
    INTR_COMP           = 137,
    INTR_LPTIM2         = 138,
    INTR_LPTIM3         = 139,
    INTR_LPTIM4         = 140,
    INTR_LPTIM5         = 141,
    INTR_LPUART         = 142,  // Global LPUART intr
    INTR_WWDG1_RST      = 143,
    INTR_CRS            = 144,
    INTR_RAMECC1_2_3    = 145,  // RAM ECC intr for RAMECC1-3
    //INTR_SAI4           = SAI4,
    // 147-148 unused
    INTR_WKUP           = 149,  // WKUP0-5 pins

};

// Wait for interrupt - enter idle mode
extern void WaitForInterrupt();

// Peripheral clocking (power enable) bits.
// All undefined bits are reserved, and must retain the reset values.
// Reset bits are the same
enum {
    // AHB1; reset 0x0000 0000
    AHB1_USB2OTGHSULPIEN = (1 << 28),
    AHB1_USB2OTGHSEN     = (1 << 27),
    AHB1_USB1OTGHSULPIEN = (1 << 26),
    AHB1_USB1OTGHSEN     = (1 << 25),
    AHB1_ETH1RXEN        = (1 << 17),
    AHB1_ETH1TXEN        = (1 << 16),
    AHB1_ETH1MACEN       = (1 << 15),
    AHB1_ADC12EN         = (1 << 5),
    AHB1_DMA2EN          = (1 << 1),
    AHB1_DMA1EN          = (1 << 0),

    // AHB2; reset 0x0000 0000
    AHB2_SRAM3EN         = (1 << 31),
    AHB2_SRAM2EN         = (1 << 30),
    AHB2_SRAM1EN         = (1 << 29),
    AHB2_SDMMC2EN        = (1 << 9),
    AHB2_RNGEN           = (1 << 6),
    AHB2_HASHEN          = (1 << 5),
    AHB2_CRYPTEN         = (1 << 4),
    AHB2_DCMIEN          = (1 << 0),

    // AHB3; reset 0x0000 0000
    AHB3_AXISRAMLPEN     = (1 << 31), // AHB3 sleep clock only (AHB3LPENR)
    AHB3_ITCMLPEN        = (1 << 30), // AHB3 sleep clock only
    AHB3_DTCM2LPEN       = (1 << 29), // AHB3 sleep clock only
    AHB3_DTCM1LPEN       = (1 << 28), // AHB3 sleep clock only
    AHB3_SDMMC1EN        = (1 << 16),
    AHB3_QSPIEN          = (1 << 14),
    AHB3_FMCEN           = (1 << 12),
    AHB3_JPGDECEN        = (1 << 5),
    AHB3_DMA2DEN         = (1 << 4),
    AHB3_MDMAEN          = (1 << 0),

    // AHB4; reset 0x0000 0000
    AHB4_SRAM4LPEN       = (1 << 29), // AHB4 sleep clock only (AHB4LPENR)
    AHB4_BKPRAMEN        = (1 << 28),
    AHB4_HSEMEN          = (1 << 25),
    AHB4_ADC3EN          = (1 << 24),
    AHB4_BDMAEN          = (1 << 21),
    AHB4_CRCEN           = (1 << 19),
    AHB4_GPIOKEN         = (1 << 10),
    AHB4_GPIOJEN         = (1 << 9),
    AHB4_GPIOIEN         = (1 << 8),
    AHB4_GPIOHEN         = (1 << 7),
    AHB4_GPIOGEN         = (1 << 6),
    AHB4_GPIOFEN         = (1 << 5),
    AHB4_GPIOEEN         = (1 << 4),
    AHB4_GPIODEN         = (1 << 3),
    AHB4_GPIOCEN         = (1 << 2),
    AHB4_GPIOBEN         = (1 << 1),
    AHB4_GPIOAEN         = (1 << 0),

    // APB1L; reset 0x0000 0000
    APB1L_UART8EN        = (1 << 31),
    APB1L_UART7EN        = (1 << 30),
    APB1L_DAC12EN        = (1 << 29),
    APB1L_CECEN          = (1 << 27),
    APB1L_I2C3EN         = (1 << 23),
    APB1L_I2C2EN         = (1 << 22),
    APB1L_I2C1EN         = (1 << 21),
    APB1L_UART5EN        = (1 << 20),
    APB1L_UART4EN        = (1 << 19),
    APB1L_USART3EN       = (1 << 18),
    APB1L_USART2EN       = (1 << 17),
    APB1L_SPDIFRXEN      = (1 << 16),
    APB1L_SPI3EN         = (1 << 15),
    APB1L_SPI2EN         = (1 << 14),
    APB1L_LPTIM1EN       = (1 << 9),
    APB1L_TIM14EN        = (1 << 8),
    APB1L_TIM13EN        = (1 << 7),
    APB1L_TIM12EN        = (1 << 6),
    APB1L_TIM7EN         = (1 << 5),
    APB1L_TIM6EN         = (1 << 4),
    APB1L_TIM5EN         = (1 << 3),
    APB1L_TIM4EN         = (1 << 2),
    APB1L_TIM3EN         = (1 << 1),
    APB1L_TIM2EN         = (1 << 0),

    // APB1H; reset 0x0000 0000
    APB1H_FDCANEN        = (1 << 8),
    APB1H_MDIOSEN        = (1 << 5),
    APB1H_OPAMPEN        = (1 << 4),
    APB1H_SWPEN          = (1 << 2),
    APB1H_CRSEN          = (1 << 1),

    // APB2; reset 0x0000 0000
    APB2_HRTIMEN         = (1 << 29),
    APB2_DFSDM1EN        = (1 << 28),
    APB2_SAI3EN          = (1 << 24),
    APB2_SAI2EN          = (1 << 23),
    APB2_SAI1EN          = (1 << 22),
    APB2_SPI5EN          = (1 << 20),
    APB2_TIM17EN         = (1 << 18),
    APB2_TIM16EN         = (1 << 17),
    APB2_TIM15EN         = (1 << 16),
    APB2_SPI4EN          = (1 << 13),
    APB2_SPI1EN          = (1 << 12),
    APB2_USART6EN        = (1 << 5),
    APB2_USART1EN        = (1 << 4),
    APB2_TIM8EN          = (1 << 1),
    APB2_TIM1EN          = (1 << 0),

    // APB3; reset 0x0000 0000
    APB3_WWDG1EN         = (1 << 6),
    APB3_LTDCEN          = (1 << 3),

    // APB4; reset 0x0001 0000
    APB4_SAI4EN          = (1 << 21),
    APB4_RTCAPBEN        = (1 << 16),
    APB4_VREFEN          = (1 << 15),
    APB4_COMP12EN        = (1 << 14),
    APB4_LPTIM5EN        = (1 << 12),
    APB4_LPTIM4EN        = (1 << 11),
    APB4_LPTIM3EN        = (1 << 10),
    APB4_LPTIM2EN        = (1 << 9),
    APB4_I2C4EN          = (1 << 7),
    APB4_SPI6EN          = (1 << 5),
    APB4_LPUART1EN       = (1 << 3),
    APB4_SYSCFGEN        = (1 << 1)

    // The LP control bits match the clock enable bits, just
    // different control regs, but verify before use.
};

#endif // __STM32H7X_H__
