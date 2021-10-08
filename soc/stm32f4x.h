// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32F4X_H__
#define __STM32F4X_H__

#define STM32F4X

#define CORTEX_M 4

#define HAVE_HW_CRC

#include "cortex-m4.h"
#include "arm.h"
#include "assert.h"
#include "bits.h"

// Max CCLK = 168MHz
enum { CCLK_MAX = 168000000 };

// On-chip STM32F405/407/415/417 peripheral base addresses
enum { 
    BASE_TIM2       = 0x40000000,
    BASE_TIM3       = 0x40000400,
    BASE_TIM4       = 0x40000800,
    BASE_TIM5       = 0x40000C00,
    BASE_TIM6       = 0x40001000,
    BASE_TIM7       = 0x40001400,
    BASE_TIM12      = 0x40001800,
    BASE_TIM13      = 0x40001C00,
    BASE_TIM14      = 0x40002000,
    BASE_RTC        = 0x40002800,
    BASE_WWDG       = 0x40002C00,
    BASE_IWDG       = 0x40003000,
    BASE_I2S2ext    = 0x40003400,
    BASE_SPI2       = 0x40003800,
    BASE_SPI3       = 0x40003C00,
    BASE_I2S3ext    = 0x40004000,
    BASE_USART2     = 0x40004400,
    BASE_USART3     = 0x40004800,
    BASE_UART4      = 0x40004C00,
    BASE_UART5      = 0x40005000,
    BASE_I2C1       = 0x40005400,
    BASE_I2C2       = 0x40005800,
    BASE_I2C3       = 0x40005C00,
    BASE_CAN1       = 0x40006400,
    BASE_CAN2       = 0x40006800,
    BASE_PWR        = 0x40007000,
    BASE_DAC        = 0x40007400,
    BASE_UART7      = 0x40007800,
    BASE_UART8      = 0x40007C00,
    BASE_TIM1       = 0x40010000,
    BASE_TIM8       = 0x40010400,
    BASE_USART1     = 0x40011000,
    BASE_USART6     = 0x40011400,
    BASE_ADC3       = 0x40012200,
    BASE_ADC2       = 0x40012100,
    BASE_ADC1       = 0x40012000,
    BASE_SDIO       = 0x40012C00,
    BASE_SPI        = 0x40013000,
    BASE_SPI4       = 0x40013400,
    BASE_SYSCFG     = 0x40013800,
    BASE_EXTI       = 0x40013C00,
    BASE_TIM9       = 0x40014000,
    BASE_TIM10      = 0x40014400,
    BASE_TIM11      = 0x40014800,
    BASE_SPI5       = 0x40015000,
    BASE_SPI6       = 0x40015400,
    BASE_SAI1       = 0x40015800,
    BASE_LCD_TFT    = 0x40016800,
    BASE_GPIOA      = 0x40020000,
    BASE_GPIOB      = 0x40020400,
    BASE_GPIOC      = 0x40020800,
    BASE_GPIOD      = 0x40020C00,
    BASE_GPIOE      = 0x40021000,
    BASE_GPIOF      = 0x40021400,
    BASE_GPIOG      = 0x40021800,
    BASE_GPIOH      = 0x40021C00,
    BASE_GPIOI      = 0x40022000,
    BASE_GPIOJ      = 0x40022400,
    BASE_GPIOK      = 0x40022800,
    BASE_CRC        = 0x40023000,
    BASE_RCC        = 0x40023800,
    BASE_FLASH      = 0x40023C00,
    BASE_BKPSRAM    = 0x40024000,
    BASE_DMA1       = 0x40026000,
    BASE_DMA2       = 0x40026400,
    BASE_ETH        = 0x40028000,
    BASE_DMA2D      = 0x4002B000,
    BASE_USB_OTG_HS = 0x40040000,

    BASE_USB_FS     = 0x50000000,
    BASE_DCMI       = 0x50050000,
    BASE_CRYP       = 0x50060000,
    BASE_HASH       = 0x50060400,
    BASE_RNG        = 0x50060800,

    BASE_FSMC       = 0xA0000000
};


// Number of peripherals and other limits
enum {
    INT_NUM   = 81,             // 81 IRQs (plus 16 fault vectors (some unused))
    EINTR_NUM = 16,
    PWM_NUM   = 2,
    GPIO_NUM  = 5,  // A-E
    UART_NUM  = 6,
    SPI_NUM   = 6,
    I2C_NUM   = 3,
    TIMER_NUM = 14,
    BKPSRAM_SIZE = 1024
};

// Interrupt IRQs
enum {
    INTR_WWDG       = 0,
    INTR_PVD        = 1,
    INTR_TAMP_STAMP = 2,
    INTR_RTC_WKUP   = 3,
    INTR_FLASH      = 4,
    INTR_RCC        = 5,
    INTR_EXTI0      = 6,
    INTR_EXTI1      = 7,
    INTR_EXTI2      = 8,
    INTR_EXTI3      = 9,
    INTR_EXTI4      = 10,
    INTR_DMA1_Stream0 = 11,
    INTR_DMA1_Stream1 = 12,
    INTR_DMA1_Stream2 = 13,
    INTR_DMA1_Stream3 = 14,
    INTR_DMA1_Stream4 = 15,
    INTR_DMA1_Stream5 = 16,
    INTR_DMA1_Stream6 = 17,
    INTR_ADC        = 18,
    INTR_CAN1_TX    = 19,
    INTR_CAN1_RX0   = 20,
    INTR_CAN1_RX1   = 21,
    INTR_CAN1_SCE   = 22,
    INTR_EXTI9_5    = 23,
    INTR_TIM1_BRK_TIM9      = 24,
    INTR_TIM1_UP_TIM10      = 25,
    INTR_TIM1_TRG_COM_TIM11 = 26,
    INTR_TIM1_CC    = 27,
    INTR_TIM2       = 28,
    INTR_TIM3       = 29,
    INTR_TIM4       = 30,
    INTR_I2C1_EV    = 31,
    INTR_I2C1_ER    = 32,
    INTR_I2C2_EV    = 33,
    INTR_I2C2_ER    = 34,
    INTR_SPI1       = 35,
    INTR_SPI2       = 36,
    INTR_USART1     = 37,
    INTR_USART2     = 38,
    INTR_USART3     = 39,
    INTR_EXTI15_10  = 40,
    INTR_RTC_Alarm  = 41,
    INTR_OTG_FS_WKUP        = 42,
    INTR_TIM8_BRK_TIM12     = 43,
    INTR_TIM8_UP_TIM13      = 44,
    INTR_TIM8_TRG_COM_TIM14 = 45,
    INTR_TIM8_CC    = 46,
    INTR_DMA1_Stream7 = 47,
    INTR_FSMC       = 48,
    INTR_SDIO       = 49,
    INTR_TIM5       = 50,
    INTR_SPI3       = 51,
    INTR_UART4      = 52,
    INTR_UART5      = 53,
    INTR_TIM6_DAC   = 54,
    INTR_TIM7       = 55,
    INTR_DMA2_Stream0 = 56,
    INTR_DMA2_Stream1 = 57,
    INTR_DMA2_Stream2 = 58,
    INTR_DMA2_Stream3 = 59,
    INTR_DMA2_Stream4 = 60,
    INTR_ETH        = 61,
    INTR_ETH_WKUP   = 62,
    INTR_CAN2_TX    = 63,
    INTR_CAN2_RX0   = 64,
    INTR_CAN2_RX1   = 65,
    INTR_CAN2_SCE   = 66,
    INTR_OTG_FS     = 67,
    INTR_DMA2_Stream5 = 68,
    INTR_DMA2_Stream6 = 69,
    INTR_DMA2_Stream7 = 70,
    INTR_USART6     = 71,
    INTR_I2C3_EV    = 72,
    INTR_I2C3_ER    = 73,
    INTR_OTG_HS_EP1_OUT = 74,
    INTR_OTG_HS_EP1_IN  = 75,
    INTR_OTG_HS_WKUP    = 76,
    INTR_OTG_HS     = 77,
    INTR_DCMI       = 78,
    INTR_CRYP       = 79,
    INTR_HASH_RNG   = 80,
    INTR_FPU        = 81
};

// Wait for interrupt - enter idle mode
extern void WaitForInterrupt();

// Peripheral clocking (power enable) bits.
// All undefined bits are reserved, and must retain the reset values.
enum {
    // AHB1; reset 0x0010 0000 = CCMDATARAMEN
    AHB1_OTGHSULPIEN = (1 << 30),
    AHB1_OTGHSEN     = (1 << 29),
    AHB1_ETHMACPTPEN = (1 << 28),
    AHB1_ETHMACRXEN  = (1 << 27),
    AHB1_ETHMACTXEN  = (1 << 26),
    AHB1_ETHMACEN    = (1 << 25),
    AHB1_DMA2EN      = (1 << 22),
    AHB1_DMA1EN      = (1 << 21),
    AHB1_CCMDATARAMEN = (1 << 20),
    AHB1_BKPSRAMEN   = (1 << 18),
    AHB1_CRCEN       = (1 << 12),
    AHB1_GPIOIEN     = (1 << 8),
    AHB1_GPIOHEN     = (1 << 7),
    AHB1_GPIOGEN     = (1 << 6),
    AHB1_GPIOFEN     = (1 << 5),
    AHB1_GPIOEEN     = (1 << 4),
    AHB1_GPIODEN     = (1 << 3),
    AHB1_GPIOCEN     = (1 << 2),
    AHB1_GPIOBEN     = (1 << 1),
    AHB1_GPIOAEN     = (1 << 0),

    // AHB2; reset 0x0000 0000
    AHB2_OTGFSEN     = (1 << 7),
    AHB2_RNGEN       = (1 << 6),
    AHB2_HASHEN      = (1 << 5),
    AHB2_CRYPEN      = (1 << 4),
    AHB2_DCMIEN      = (1 << 0),

    // AHB3; reset 0x0000 0000
    AHB3_FSMCEN      = (1 << 0),

    // APB1; reset 0x0000 0000
    APB1_DACEN       = (1 << 29),
    APB1_PWREN       = (1 << 28),
    APB1_CAN2EN      = (1 << 26),
    APB1_CAN1EN      = (1 << 25),
    APB1_I2C3EN      = (1 << 23),
    APB1_I2C2EN      = (1 << 22),
    APB1_I2C1EN      = (1 << 21),
    APB1_UART5EN     = (1 << 20),
    APB1_UART4EN     = (1 << 19),
    APB1_USART3EN    = (1 << 18),
    APB1_USART2EN    = (1 << 17),
    APB1_SPI3EN      = (1 << 15),
    APB1_SPI2EN      = (1 << 14),
    APB1_WWDGEN      = (1 << 11),
    APB1_TIM14EN     = (1 << 8),
    APB1_TIM13EN     = (1 << 7),
    APB1_TIM12EN     = (1 << 6),
    APB1_TIM7EN      = (1 << 5),
    APB1_TIM6EN      = (1 << 4),
    APB1_TIM5EN      = (1 << 3),
    APB1_TIM4EN      = (1 << 2),
    APB1_TIM3EN      = (1 << 1),
    APB1_TIM2EN      = (1 << 0),

    // APB2; reset 0x0000 0000
    APB2_SPI6EN      = (1 << 21),
    APB2_API5EN      = (1 << 20),
    APB2_TIM11EN     = (1 << 18),
    APB2_TIM10EN     = (1 << 17),
    APB2_TIM9EN      = (1 << 16),
    APB2_SYSCFGEN    = (1 << 14),
    APB2_SPI1EN      = (1 << 12),
    APB2_SDIOEN      = (1 << 11),
    APB2_ADC3EN      = (1 << 10),
    APB2_ADC2EN      = (1 << 9),
    APB2_ADC1EN      = (1 << 8),
    APB2_USART6EN    = (1 << 5),
    APB2_USART1EN    = (1 << 4),
    APB2_TIM8EN      = (1 << 1),
    APB2_TIM1EN      = (1 << 0)
};

#endif // __STM32F4X_H__
