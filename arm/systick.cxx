// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "systick.h"


void SysTick::SetTimer(uint usec) {
    Thread::IPL G(IPL_SYSTICK-1);

    // 1000000/976 = 1024, which reduces the division to a bit shift. 
    // The max value here is 34932us = 34.9ms before the multiplication
    // overflows.
    const uint32_t count = (CCLK/976) * usec / uint(1000000/976);

    SYST_RVR = count;
    SYST_CVR = 0;  // Forces reload
    SYST_CSR = CSR_CLKSOURCE | CSR_TICKINT | CSR_ENABLE;
}


void SysTick::Interrupt(void*) {
    if (SYST_CSR & CSR_COUNTFLAG) {
        SYST_CSR &= ~(CSR_TICKINT | CSR_ENABLE);
        Thread::TimerInterrupt();
    }
}
