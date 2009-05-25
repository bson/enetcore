#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps SoC and board peripherals to canonical names and
// provides external decls.

#include "lpc_gpio.h"
#include "lpc_i2c.h"
#include "lpc_uart.h"
#include "lpc_spi.h"
#include "lpc_timer.h"
#include "lpc_eintr.h"

// SoC peripherals

typedef LpcEintr Eintr;
typedef LpcI2cBus I2cBus;
typedef LpcI2cDev I2cDev;
typedef LpcUart SerialPort;
typedef LpcSpiBus SpiBus;
typedef LpcSpiDev SpiDev;
typedef LpcTimer Timer;
typedef LpcGpio Gpio;

extern Vic _vic;

extern I2cBus _i2c0;
extern SerialPort _uart0, _uart1;
extern SpiBus _spi0, _spi1;
extern Gpio _gpio[2];
extern Eintr _eintr0, _eintr1, _eintr2, _eintr3;

// Board peripherals

#define _console _uart0
#define _lcd _uart1

#include "cs8900a.h"
typedef MacCS8900a Ethernet;

extern Ethernet _eth0;

#endif // __BOARD_H__
