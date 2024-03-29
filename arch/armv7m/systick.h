// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

#include "compiler.h"
#include "core/bits.h"

// SYSTICK core timer used by scheduler.
// Ticks at CCLK for up to (2^32-1)/CCLK.  At 120MHz, that's 35s.
// Stops when run down and needs to be reloaded.

class SysTick {
    enum {
        CSR_COUNTFLAG = BIT16,
        CSR_CLKSOURCE = BIT2,
        CSR_TICKINT   = BIT1,
        CSR_ENABLE    = BIT0
    };

public:
    SysTick() { }

    static void Interrupt(void*);
    void SetTimer(uint32_t usec);
};

typedef SysTick SysTimer;

#endif  // _SYSTICK_H_
