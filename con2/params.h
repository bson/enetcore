// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __PARAMS_H__
#define __PARAMS_H__

#undef ENABLE_ENET
#undef ENABLE_USB
#undef ENABLE_PANEL
#undef ENABLE_IP

#define ENABLE_WFI

enum { 
    FOSC          = 12000000,   // Crystal = 12MHz
    HCLK          = 168000000,  // HCLK/core
    APB1_CLK      = 42000000,   // APB1
    APB1_TIMERCLK = 42000000,   // APB1 timer clock
    APB2_CLK      = 84000000,   // APB2 bus
    APB2_TIMERCLK = 84000000,   // APB2 timer clock
    CLOCK_TICK    = 42000000,   // System clock (TIM5) tick: 42MHz
    CLOCK_HZ      = 1,          // Reload frequency for system cl
};

enum {
    CCLK = HCLK                 // For SysTick
};

enum {
    I2C_BUS_SPEED = 100000,
};

#define PACKAGE_PINS 100


#endif // __PARAMS_H__
