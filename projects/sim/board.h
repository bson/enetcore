// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __BOARD_H__
#define __BOARD_H__

#include "compiler.h"
#include "params.h"
#include "config.h"
#include "soc/stm32h7x/h753.h"

// Core and SoC resources used

typedef Stm32ClockTree ClockTree;
typedef Stm32Power Power;
typedef Stm32Flash Flash;
typedef Stm32Dma Dma;
typedef Stm32Rtc Rtc;
typedef Stm32GpioPort Gpio;
#define Uart Stm32Usart
typedef Stm32Timer<uint32_t> Timer32;
typedef Stm32Timer<uint16_t> Timer16;
typedef Stm32Random Random;
typedef Stm32Debug Debug;
typedef Stm32Eintr Eintr;
typedef Stm32Fmc Fmc;

extern Clock _clock;
extern NVic _nvic;
//extern Swo _swo;
extern Uart<UART7_SENDQ_SIZE, UART7_RECVQ_SIZE> _uart7;
extern Uart<UART5_SENDQ_SIZE, UART5_SENDQ_SIZE> _uart5;

extern Gpio _gpio_a;
extern Gpio _gpio_b;
extern Gpio _gpio_c;
extern Gpio _gpio_d;
extern Gpio _gpio_e;

extern Dma _dma1;
extern Timer16 _tim5;

// Board peripherals
// LAN8720A

#endif // __BOARD_H__
