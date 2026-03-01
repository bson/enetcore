// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __PARAMS_H__
#define __PARAMS_H__

#undef ENABLE_SWO

#undef ENABLE_ENET
#undef ENABLE_USB
#undef ENABLE_IP


#define MHZ(F) ((F)*1000000)

enum { 
    CPU_FREQ      = MHZ(240),
    AHB_FREQ      = MHZ(120),
    APB1_FREQ     = MHZ(20),    // APB1
    APB2_FREQ     = APB1_FREQ,  // APB2
    APB3_FREQ     = APB1_FREQ,  // APB3 
    APB4_FREQ     = APB1_FREQ,  // APB4
    CLOCK_TICK    = APB2_FREQ,  // System clock (TIM5) tick
    CLOCK_HZ      = 1,          // Reload frequency for system clock
};

enum {
    CCLK = CPU_FREQ,                // Core (CPU) clock
    SYSTICK_CLK = CPU_FREQ/8        // Systick clock (alternative, HCLK/8)
};
              
#define PACKAGE_PINS 144

// SysTick uses alternative, implementation specific clock
#define SYSTICK_ALT

#endif // __PARAMS_H__
