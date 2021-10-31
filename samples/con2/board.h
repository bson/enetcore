// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps SoC and board peripherals to canonical names and
// provides external decls.

#include "params.h"
#include "soc/stm32f4x/f405.h"

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
typedef Stm32Fsmc Fsmc;

//typedef Stm32SpiBus SpiBus;
//typedef Stm32SpiDev SpiDev;

extern NVic _nvic;

extern Uart<USART3_SENDQ_SIZE, USART3_RECVQ_SIZE> _usart3;
extern Uart<UART4_SENDQ_SIZE, UART4_SENDQ_SIZE> _uart4;
//extern SpiBus _spi0;

extern Gpio _gpio_a;
extern Gpio _gpio_b;
extern Gpio _gpio_c;
extern Gpio _gpio_d;
extern Gpio _gpio_e;

extern Dma _dma1;
extern Dma _dma2;
extern Timer16 _tim6;
extern Dac _dac;

// Board peripherals

#ifdef ENABLE_PANEL
#include "devices/ssd1963.h"
// XXX
// #include "tsc2046.h"
#include "panel_accessor.h"
#endif

#define _console _usart3

#ifdef ENABLE_PANEL
typedef ssd1963::Panel<PanelAccessor> Panel;
extern Panel _panel;
template class ssd1963::Panel<PanelAccessor>;
// XXX
// using namespace tsc2046;
// extern TouchController _touch;
#endif

#endif // __BOARD_H__
