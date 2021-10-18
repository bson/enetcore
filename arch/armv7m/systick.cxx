// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#include "params.h"
#include "enetkit.h"
#include "systick.h"


void SysTick::SetTimer(uint32_t usec) {
    // The PREC_ENH is a precision enhancer.
    // For example, if we have SYSTICK_CLK of 168MHz we get a max wait of about 200s.
    // Count is 21/us.
    // If we time for say 100us this is not a problem.  But if we time for 10s we get
    // a precision of 10/21 = ~0.5s!  That's a pretty large rounding error.
    // If instead we have a precision enhancement of 100, the 1/21 rounding error
    // turns into 1/21% or just under 0.05%.  The flip side is we shorten the max
    // timing interval, to avoid having to perform a 64-bit multiply here.  Of course, 
    // instead of 100 we use 128 which reduces to a shift in the final scale-down.
    enum { 
        PREC_ENH       = 128,
        MAX_WAIT_USEC  = ~(uint32_t)0 / uint32_t(SYSTICK_CLK) * 1000000UL / uint32_t(PREC_ENH),
        COUNT_PER_USEC = uint32_t(SYSTICK_CLK) * uint32_t(PREC_ENH) / 1000000UL
    };

    static_assert((uint32_t)COUNT_PER_USEC != 0);
    static_assert((uint32_t)COUNT_PER_USEC <= ~(uint16_t)0);
    static_assert((uint32_t)MAX_WAIT_USEC > 10000); // At least 10ms

    const uint32_t count = (uint32_t)(usec > MAX_WAIT_USEC ? MAX_WAIT_USEC : usec)
        * (uint16_t)COUNT_PER_USEC / PREC_ENH;

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
