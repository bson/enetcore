#ifndef __STM32_POWER__
#define __STM32_POWER__

class Stm32Power {
    enum {
        VOS = (1 << 14)
    };

public:
    // Not thread or interrupt safe.
    static void EnableVos() {
        *(volatile uint32_t*)BASE_PWR |= VOS;
    }

    // Not thread or interrupt safe.
    static void DisableVos() {
        *(volatile uint32_t*)BASE_PWR &= ~VOS;
    }
};

#endif // __STM32_POWER__
