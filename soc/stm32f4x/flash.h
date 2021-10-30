#ifndef __STM32_FLASH_H__
#define __STM32_FLASH_H__

#include "core/bits.h"

// Basic flash support.  No erase or write support.
class Stm32Flash {
    enum Register {
        FLASH_ACR = 0x00
    };

public:
    enum {
        // FLASH_ACR, reset 0x0000 0000
        DCRST   = 12,
        ICRST   = 11,
        DCEN    = 10,
        ICEN    = 9,
        PRFTEN  = 8,
        LATENCY = 0,
    };

private:
    template <typename T>
    static T& reg(const Register r) { return *((T*)(BASE_FLASH+(uint32_t)r)); }

public:
    static void Latency(uint32_t latency) {
        reg<volatile uint32_t>(Register::FLASH_ACR) |= BIT(DCRST) | BIT(ICRST);
        reg<volatile uint32_t>(Register::FLASH_ACR) = (reg<const uint32_t>(Register::FLASH_ACR) & ~(3 << LATENCY))
            | BIT(DCEN) | BIT(ICEN) | BIT(PRFTEN) | (latency << LATENCY);
    }

    static void EnableIDCaching() {
        reg<volatile uint32_t>(Register::FLASH_ACR) |= BIT(ICEN) | BIT(DCEN);
    }
};

#endif // __STM32_FLASH_H__
