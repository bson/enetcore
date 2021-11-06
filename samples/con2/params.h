// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __PARAMS_H__
#define __PARAMS_H__

#define ENABLE_PANEL
#undef ENABLE_SWO
#undef ENABLE_ESP12

#undef ENABLE_ENET
#undef ENABLE_USB
#undef ENABLE_IP

#define ENABLE_WFI

enum { 
    FOSC          = 12000000,   // Crystal = 12MHz
    HCLK          = 168000000,  // HCLK/core
    APB1_CLK      = 42000000,   // APB1
    APB1_TIMERCLK = APB1_CLK*2, // APB1 timer clock
    APB2_CLK      = 84000000,   // APB2 bus
    APB2_TIMERCLK = APB2_CLK*2, // APB2 timer clock
    CLOCK_TICK    = APB1_TIMERCLK, // System clock (TIM5) tick
    CLOCK_HZ      = 1,          // Reload frequency for system clock
};

enum {
    CCLK = HCLK,                // Core (CPU) clock

    // At 168MHz we can set a scheduling timer for a max of 2**32 / 168M * 8 = 200s
    // There are 168/8 = 21 ticks per usec, giving us slightly better than 50ns precision.
    // With a significantly slower HCLK, we'd want to undefine SYSTICK_ALT below and use
    // the full HCLK here.
    SYSTICK_CLK = HCLK/8        // Systick clock (alternative, HCLK/8)
};
              
#define PACKAGE_PINS 100

// SysTick uses alternative, implementation specific clock
#define SYSTICK_ALT

#endif // __PARAMS_H__
