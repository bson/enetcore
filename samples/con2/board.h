// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps SoC and board peripherals to canonical names and
// provides external decls.

#include "params.h"
#include "stm32f4x/f405.h"

// SoC peripherals used on CON2

typedef Stm32ClockTree ClockTree;
typedef Stm32Power Power;
typedef Stm32Flash Flash;
typedef Stm32Dma Dma;
typedef Stm32Rtc Rtc;
typedef Stm32GpioPort Gpio;
#define Uart Stm32Usart
typedef Stm32Timer<uint32_t> Timer32;
typedef Stm32Timer<uint16_t> Timer16;
typedef Stm32Adc Adc;
typedef Stm32AdcCommon AdcCommon;
typedef Stm32Dac Dac;
typedef Stm32Random Random;
typedef Stm32Debug Debug;

//typedef Stm32Fsmc Fsmc;
//typedef Stm32Eintr Eintr;
//typedef Stm32I2cBus I2cBus;
//typedef Stm32I2cDev I2cDev;
//typedef Stm32SpiBus SpiBus;
//typedef Stm32SpiDev SpiDev;
//typedef Stm32GpioIntr GpioIntr;

extern NVic _nvic;

//extern I2cBus _i2c2;
extern Uart<USART3_SENDQ_SIZE, USART3_RECVQ_SIZE> _usart3;
extern Uart<UART4_SENDQ_SIZE, UART4_SENDQ_SIZE> _uart4;
//extern SpiBus _spi0;

extern Gpio _gpio_a;
extern Gpio _gpio_b;
extern Gpio _gpio_c;
extern Gpio _gpio_d;
extern Gpio _gpio_e;
//extern GpioIntr _gpio0_intr;
//extern GpioIntr _gpio2_intr;

extern Dac _dac;

// Board peripherals

#ifdef ENABLE_PANEL
#include "ssd1963.h"
#include "tsc2046.h"
#endif

#define _console _usart3

#ifdef ENABLE_PANEL
typedef ssd1963::Panel<_gpio2, _gpio0, BIT18, BIT15, BIT17, BIT16> Panel;
extern Panel _panel;
template class ssd1963::Panel<_gpio2, _gpio0, BIT18, BIT15, BIT17, BIT16>;
using namespace tsc2046;
extern TouchController _touch;
#endif

#endif // __BOARD_H__
