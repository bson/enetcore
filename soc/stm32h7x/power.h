#ifndef __STM32_POWER_H__
#define __STM32_POWER_H___


class Stm32Power {

public:
    // Not thread or interrupt safe.  Enable voltage scaling.
    static void EnableVos() {
    }

    // Not thread or interrupt safe.  Disable voltage scaling.
    static void DisableVos() {
    }
};

#endif // __STM32_POWER_H__
