#ifndef __STM32_DEBUG_H__
#define __STM32_DEBUG_H__

#include "bits.h"

class Stm32Debug {
public:
    enum Register {
        DBGMCU_APB1_FZ = 0xe0042008,
        DBGMCU_APB2_FZ = 0xe004200c
    };

    enum {
        // DBG_APB1_FZ
        APB1_IWDT_STOP = BIT(12),
        APB1_WWDT_STOP = BIT(11),
        APB1_RTC_STOP  = BIT(10),
        APB1_TIM14_STOP = BIT(8),
        APB1_TIM13_STOP = BIT(7),
        APB1_TIM12_STOP = BIT(6),
        APB1_TIM7_STOP = BIT(5),
        APB1_TIM6_STOP = BIT(4),
        APB1_TIM5_STOP = BIT(3),
        APB1_TIM4_STOP = BIT(2),
        APB1_TIM3_STOP = BIT(1),
        APB1_TIM2_STOP = BIT(0),

        // DBGMCU_APB2_FZ
        APB2_TIM11_STOP = BIT(18),
        APB2_TIM10_STOP = BIT(17),
        APB2_TIM9_STOP  = BIT(16),
        APB2_TIM8_STOP  = BIT(1),
        APB2_TIM1_STOP  = BIT(0)
    };
        
    template <typename T>
    static T& reg(uint32_t addr) { return *(T*)addr; }

    static void FreezeAPB1(uint32_t bits) {
        reg<volatile uint32_t>(DBGMCU_APB1_FZ) |= bits;
    }

    static void FreezeAPB2(uint32_t bits) {
        reg<volatile uint32_t>(DBGMCU_APB2_FZ) |= bits;
    }
};

#endif // __STM32_DEBUG_H__
