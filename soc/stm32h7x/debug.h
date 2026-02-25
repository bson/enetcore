// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_DEBUG_H__
#define __STM32_DEBUG_H__

#include <stdint.h>
#include "core/bits.h"

class Stm32Debug {
public:
    enum Register {
        // Peripheral freeze registers
        APB1_FZ = 0x03c,
        APB2_FZ = 0x04c,
        APB3_FZ = 0x034,
    };

    enum {
        // APB1_FZ
        I2C3   = 23,
        I2C2   = 22,
        I2C1   = 21,
        LPTIM1 = 9,
        TIM14  = 8,
        TIM13  = 7,
        TIM12  = 6,
        TIM7   = 5,
        TIM6   = 4,
        TIM5   = 3,
        TIM4   = 2,
        TIM3   = 1,
        TIM2   = 0,

        // APB2_FZ
        HRTIM  = 29,
        TIM17  = 18,
        TIM16  = 17,
        TIM15  = 16,
        TIM8   = 1,
        TIM1   = 0,

        // APB3_FZ
        WWDG1   = 6,
    };
        
    static volatile uint32_t& reg(uint32_t offset) {
        return *(volatile uint32_t*)(BASE_DBGMCU + offset);
    }

    // Peripheral freeze-on-debug is only set once, during init, hence
    // no unfreeze methods.
    static void FreezeAPB1(uint32_t bits) { reg(APB1_FZ) |= bits; }
    static void FreezeAPB2(uint32_t bits) { reg(APB2_FZ) |= bits; }
    static void FreezeAPB3(uint32_t bits) { reg(APB3_FZ) |= bits; }
};

#endif // __STM32_DEBUG_H__
