// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps SoC and board peripherals to canonical names and
// provides external decls.

#include "params.h"

#include "stm32f4x.h"

#include "nvic.h"
#include "systick.h"
#include "stm32f4x/power.h"
#include "stm32f4x/clock_tree.h"
#include "stm32f4x/debug.h"
#include "stm32f4x/flash.h"
#include "stm32f4x/syscfg.h"
#include "stm32f4x/gpio.h"
#include "stm32f4x/usart.h"
#include "stm32f4x/random.h"
#include "stm32f4x/timer.h"
#include "stm32f4x/backup.h"
#include "stm32f4x/rtc.h"

//#include "stm32f4x/crc.h"
//#include "stm32f4x/i2c.h"
//#include "stm32f4x/spi.h"
//#include "stm32f4x/eintr.h"
//#include "stm32f4x/pwm.h"

// SoC peripherals used on CON2

typedef Stm32Rtc Rtc;
typedef Stm32GpioPort Gpio;
typedef Stm32Usart SerialPort;
typedef Stm32Timer<uint32_t> Timer32;
typedef Stm32Timer<uint16_t> Timer;

//typedef Stm32Eintr Eintr;
//typedef Stm32I2cBus I2cBus;
//typedef Stm32I2cDev I2cDev;
//typedef Stm32SpiBus SpiBus;
//typedef Stm32SpiDev SpiDev;
//typedef Stm32GpioIntr GpioIntr;
//typedef Stm32Pwm Pwm;

extern NVic _nvic;

//extern I2cBus _i2c2;
extern SerialPort _usart3;
extern SerialPort _uart4;
//extern SpiBus _spi0;

extern Gpio _gpio_a;
extern Gpio _gpio_b;
extern Gpio _gpio_c;
extern Gpio _gpio_d;
extern Gpio _gpio_e;
//extern GpioIntr _gpio0_intr;
//extern GpioIntr _gpio2_intr;

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