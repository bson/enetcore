// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#include "params.h"
#include "enetkit.h"
#include "systick.h"


void SysTick::SetTimer(uint32_t usec) {
    enum { 
        MAX_WAIT_USEC = ~(uint32_t)0 / SYSTICK_CLK,
        COUNT_PER_USEC = SYSTICK_CLK / 1000000
    };

    static_assert((uint32_t)COUNT_PER_USEC != 0);
    static_assert((uint32_t)COUNT_PER_USEC <= ~(uint16_t)0);

    const uint32_t count = usec > MAX_WAIT_USEC ? MAX_WAIT_USEC : usec * (uint16_t)COUNT_PER_USEC;

    Thread::IPL G(IPL_SYSTICK);

    SYST_RVR = count;
    SYST_CVR = 0;  // Forces reload
#ifdef SYSTICK_ALT
    SYST_CSR = CSR_TICKINT | CSR_ENABLE;
#else
    SYST_CSR = CSR_CLKSOURCE | CSR_TICKINT | CSR_ENABLE;
#endif
}


void SysTick::Interrupt(void*) {
    if (SYST_CSR & CSR_COUNTFLAG) {
        SYST_CSR &= ~(CSR_TICKINT | CSR_ENABLE);
        Thread::TimerInterrupt();
    }
}
