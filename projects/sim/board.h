// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps SoC and board peripherals to canonical names and
// provides external decls.

#include "params.h"
#include "soc/stm32h7x/h753.h"

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
typedef Stm32Random Random;
typedef Stm32Debug Debug;
typedef Stm32Eintr Eintr;

extern NVic _nvic;

extern Swo _swo;
extern Uart<USART3_SENDQ_SIZE, USART3_RECVQ_SIZE> _usart3;
extern Uart<UART4_SENDQ_SIZE, UART4_SENDQ_SIZE> _uart4;

extern Gpio _gpio_a;
extern Gpio _gpio_b;
extern Gpio _gpio_c;
extern Gpio _gpio_d;
extern Gpio _gpio_e;

extern Dma _dma1;
extern Timer16 _tim6;

// Board peripherals


#endif // __BOARD_H__
