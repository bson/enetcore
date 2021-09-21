#ifndef __STM32_POWER__
#define __STM32_POWER__

class Stm32Power {
    enum {
        PWR_CR_VOS = (1 << 14)
    };

    // Not thread or interrupt safe.
    static void EnableVos() {
        *(volatile uint32*)BASE_PWR |= PWR_CR_VOS;
    }

    // Not thread or interrupt safe.
    static void DisableVos() {
        *(volatile uint32*)BASE_PWR &= ~PWR_CR_VOS;
    }
};

#endif // __STM32_POWER__
