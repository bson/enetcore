#ifndef __STM32_POWER_H__
#define __STM32_POWER_H___

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

#endif // __STM32_POWER_H__
