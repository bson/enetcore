// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __H753_H__
#define __H753_H__

enum class H753MemSize {
    SIZE_FLASHB1  = 1024*1024,
    SIZE_FLASHB2  = 1024*1024,

    SIZE_DTCM     = 128*1024,

    SIZE_SRAM1    = 128*1024,
    SIZE_SRAM2    = 128*1024,
    SIZE_SRAM3    = 32*1024,
    SIZE_SRAM4    = 64*1024,
    SIZE_BKPSRAM  = 4*1024,

    SIZE_AXI_SRAM = 512*1024,
};


#include "soc/stm32h7x.h"

#include "arch/armv7m/nvic.h"
#include "arch/armv7m/systick.h"
#include "arch/armv7m/swo.h"

#include "soc/stm32h7x/power.h"
#include "soc/stm32h7x/clock_tree.h"
#include "soc/stm32h7x/debug.h"
#include "soc/stm32h7x/flash.h"
#include "soc/stm32h7x/syscfg.h"
#include "soc/stm32h7x/dma.h"
#include "soc/stm32h7x/gpio.h"
#include "soc/stm32h7x/usart.h"
#include "soc/stm32h7x/random.h"
#include "soc/stm32h7x/timer.h"
#include "soc/stm32h7x/backup.h"
#include "soc/stm32h7x/rtc.h"
//#include "soc/stm32h7x/adc.h"
#include "soc/stm32h7x/dac.h"
#include "soc/stm32h7x/fmc.h"
#include "soc/stm32h7x/eintr.h"
//#include "soc/stm32h7x/spi.h"

#include "soc/stm32h7x/crc.h"
//#include "soc/stm32h7x/i2c.h"
//#include "soc/stm32h7x/ethernet.h
//#include "soc/sdmmc.h"
#include "eeprom.h"
//#include "soc/stm32h7x/usb_dev.h"

#include "soc/hash.h"
//#include "soc/crypt.h"

#endif // __H753_H__
