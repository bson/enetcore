#ifndef __FPU_H__
#define __FPU_H__

#include "core/bits.h"


extern volatile uint32_t CPACR;
extern volatile uint32_t FPCCR;

class Fpu {
    enum {
        // CPACR
        CP10 = 20,
        CP11 = 22,

        // FPCCR
        ASPEN = 31,
        LSPEN = 30
    };

public:
    static void EnableAccess() {
        // Disable automatic or lazy FP register saves.  Enetcore
        // saves them on CSW.
        FPCCR &= ~(BIT(ASPEN) | BIT(LSPEN));

        // Enable access
        CPACR |= (0b11 << CP10) | (0b11 << CP11);
    }
};

#endif // __FPU_H__
