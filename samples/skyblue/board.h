// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps SoC and board peripherals to canonical names and
// provides external decls.

#define ENABLE_ENET
#undef ENABLE_USB
#define ENABLE_PLL_CLK          // Undefine to run at 1:1 with crystal, at 12MHz
#define ENABLE_PANEL

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

#define PACKAGE_PINS 80

#include "lpc4078.h"

#include "nvic.h"
#include "systick.h"
#include "lpc_crc.h"
#include "lpc_pll.h"
#include "lpc_gpio.h"
#include "lpc_i2c.h"
#include "lpc_uart.h"
#include "lpc_spi.h"
#include "lpc_timer.h"
#include "lpc_eeprom.h"
#include "lpc_eintr.h"
#include "lpc_pwm.h"
#include "lpc_usb_dev.h"
#include "tlk110.h"
#include "lpc_ethernet.h"

// SoC peripherals

typedef LpcPll Pll;
typedef LpcEintr Eintr;
typedef LpcI2cBus I2cBus;
typedef LpcI2cDev I2cDev;
typedef LpcUart SerialPort;
typedef LpcSpiBus SpiBus;
typedef LpcSpiDev SpiDev;
typedef LpcTimer Timer;
typedef LpcGpio Gpio;
typedef LpcGpioIntr GpioIntr;
typedef LpcEeprom Eeprom;
typedef LpcPwm Pwm;
typedef LpcUsbDev Usb;

extern Pll _pll0;
extern Pll _pll1;
extern NVic _nvic;

extern Eeprom _eeprom;
extern I2cBus _i2c2;
extern SerialPort _uart3;
extern SpiBus _spi0;
extern Gpio _gpio0;
extern Gpio _gpio1;
extern Gpio _gpio2;
extern Gpio _gpio4;
extern GpioIntr _gpio0_intr;
extern GpioIntr _gpio2_intr;

#ifdef ENABLE_USB
extern Usb _usb;
#endif
#ifdef ENABLE_ENET
extern Ethernet _eth0;
#endif

// Board peripherals

#ifdef ENABLE_PANEL
#include "ssd1963.h"
#include "tsc2046.h"
#endif

#include "sdcard.h"
#include "fat.h"

#define _console _uart3

#ifdef ENABLE_PANEL
typedef ssd1963::Panel<_gpio2, _gpio0, BIT18, BIT15, BIT17, BIT16> Panel;
extern Panel _panel;
template class ssd1963::Panel<_gpio2, _gpio0, BIT18, BIT15, BIT17, BIT16>;
using namespace tsc2046;
extern TouchController _touch;
#endif

extern SpiDev _cardslot;
extern SDCard _sd;
extern Fat _fat;

#endif // __BOARD_H__
