// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __PARAMS_H__
#define __PARAMS_H__

#undef ENABLE_SWO

#undef ENABLE_ENET
#undef ENABLE_USB
#undef ENABLE_IP


enum { 
    CPU_FREQ      = 200000000,
    AHB_FRQ       = 100000000,
    APB1_FREQ     =  20000000,   // APB1
    APB2_FREQ     =  20000000,   // APB2
    APB3_FREQ     =  20000000,   // APB3 
    CLOCK_TICK    = APB2_FREQ,   // System clock (TIM5) tick
    CLOCK_HZ      = 1,           // Reload frequency for system clock
};

enum {
    CCLK = CPU_FREQ,                // Core (CPU) clock
    SYSTICK_CLK = CPU_FREQ/8        // Systick clock (alternative, HCLK/8)
};
              
#define PACKAGE_PINS 144

// SysTick uses alternative, implementation specific clock
#define SYSTICK_ALT

#endif // __PARAMS_H__
