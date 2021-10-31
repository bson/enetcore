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

enum {
    I2C_BUS_SPEED = 100000,
};

// Panel is SRAM in bank 2 (/CE2)
#define PANEL_BANK Fsmc::Bank::BANK2

// FSMC SRAM mode 1 timing
enum {
    PANEL_BUS_FREQ       = 10000000,  // 10MHz bus freq (5MHz write/read speed)      
    PANEL_HCLK_PER_CYC   = HCLK/PANEL_BUS_FREQ,                 // HCLKs per bus cycle
    PANEL_BUS_TURN_CLK   = PANEL_HCLK_PER_CYC/2,                // Bus turn HCLKs
    PANEL_DATA_CLK       = (PANEL_HCLK_PER_CYC - PANEL_BUS_TURN_CLK), // Data cycle HCLKs
    PANEL_DATA_SETUP_CLK = int(HCLK*4.5e-12)+1, // Data setup (actually address setup for data access)
    PANEL_DATA_HOLD_CLK = PANEL_DATA_CLK - PANEL_DATA_SETUP_CLK // Data hold, HCLKs
};

// Timing specs from SSD1963 datasheet
static_assert((float)PANEL_BUS_TURN_CLK / (float)HCLK >= 9e-12);   // >= 9ns bus turn
static_assert((float)PANEL_DATA_CLK / (float)HCLK >= 9e-12);       // >= 9ns data cycle
static_assert((float)PANEL_DATA_HOLD_CLK / (float)HCLK >= 1e-12);  // >= 1ns data hold
static_assert((float)PANEL_DATA_SETUP_CLK / (float)HCLK >= 4e-12);  // >= 4ns data setup

static_assert(PANEL_DATA_HOLD_CLK >= 1);
static_assert(PANEL_DATA_HOLD_CLK <= 255);
static_assert(PANEL_DATA_SETUP_CLK <= 15);
              
#define PACKAGE_PINS 100

// SysTick uses alternative, implementation specific clock
#define SYSTICK_ALT

#endif // __PARAMS_H__
