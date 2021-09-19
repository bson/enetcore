// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps SoC and board peripherals to canonical names and
// provides external decls.

#undef ENABLE_ENET
#undef ENABLE_USB
#define ENABLE_PLL_CLK          // Undefine to run at 1:1 with crystal, at 12MHz
#undef ENABLE_PANEL

#define ENABLE_WFI

enum { 
    FOSC = 12000000,            // Crystal = 12MHz
};

#ifdef ENABLE_PLL_CLK
enum {
    CCLK = 120000000,           // CCLK = 120MHz
    PCLK = CCLK/4,              // PCLK when run off pll_clk, must be CCLK / 1-4
};
#else
// Simplified clocking for JTAG use.  Run CLK = FOSC; PCLK=CCLK/2, disable PLL0
enum { 
    CCLK = FOSC,
    PCLK = CCLK/2
};
#endif

enum {
    I2C_BUS_SPEED = 100000,
    USBCLK = 48000000,
};

#define PACKAGE_PINS 100

#include "stm32f405.h"

#include "nvic.h"
#include "systick.h"
//#include "stm_crc.h"
//#include "stm_pll.h"
#include "stm_gpio.h"
//#include "stm_i2c.h"
//#include "stm_uart.h"
//#include "stm_spi.h"
//#include "stm_timer.h"
//#include "stm_eeprom.h"
//#include "stm_eintr.h"
//#include "stm_pwm.h"

// SoC peripherals

//typedef StmPll Pll;
//typedef StmEintr Eintr;
//typedef StmI2cBus I2cBus;
//typedef StmI2cDev I2cDev;
//typedef StmUart SerialPort;
//typedef StmSpiBus SpiBus;
//typedef StmSpiDev SpiDev;
//typedef StmTimer Timer;
typedef StmGpio Gpio;
typedef StmGpioIntr GpioIntr;
//typedef StmEeprom Eeprom;
//typedef StmPwm Pwm;

extern Pll _pll0;
extern Pll _pll1;
extern NVic _nvic;

//extern Eeprom _eeprom;
//extern I2cBus _i2c2;
//extern SerialPort _uart3;
//extern SpiBus _spi0;
//extern Gpio _gpio_a;
//extern Gpio _gpio_b;
//extern Gpio _gpio_c;
//extern Gpio _gpio_d;
//extern Gpio _gpio_e;
//extern GpioIntr _gpio0_intr;
//extern GpioIntr _gpio2_intr;

// Board peripherals

#ifdef ENABLE_PANEL
#include "ssd1963.h"
#include "tsc2046.h"
#endif

//#define _console _uart3

#ifdef ENABLE_PANEL
typedef ssd1963::Panel<_gpio2, _gpio0, BIT18, BIT15, BIT17, BIT16> Panel;
extern Panel _panel;
template class ssd1963::Panel<_gpio2, _gpio0, BIT18, BIT15, BIT17, BIT16>;
using namespace tsc2046;
extern TouchController _touch;
#endif

extern SpiDev _cardslot;

#endif // __BOARD_H__
