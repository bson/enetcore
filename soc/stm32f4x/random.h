#ifndef __STM32_RANDOM_H__
#define __STM32_RANDOM_H__

#include "core/bits.h"

class Stm32Random {
    enum class Register {
        RNG_CR = 0x00,
        RNG_SR = 0x04,
        RNG_DR = 0x08
    };

    enum {
        // RNG_CR
        IE    = 3,
        RNGEN = 2,

        // RNG_SR
        SEIS = 6,
        CEIS = 5,
        SECS = 2,
        CECS = 1,
        DRDY = 0
    };
    
    template <typename T>
    static T& reg(const Register r) {
        return *((T*)(BASE_RNG + (uint32_t)r)); 
    }

public:
    // Start RNG generator.  Returns true if okay.
    static bool Init() {
        volatile uint32_t& cr = reg<volatile uint32_t>(Register::RNG_CR);
        cr &= BIT(IE);
        cr |= BIT(RNGEN);
        (void)Random();

        volatile uint32_t& sr = reg<volatile uint32_t>(Register::RNG_SR);
        return (sr & (BIT(SECS) | BIT(SECS))) == 0;
    }

    // Busy-wait and return the next random value.
    static uint32_t Random() {
        volatile uint32_t& sr = reg<volatile uint32_t>(Register::RNG_SR);
        while (!(sr & BIT(DRDY)))
            ;
        return reg<volatile uint32_t>(Register::RNG_DR);
    }

    // Generate a sequence of random bytes
    static void Random(uint8_t* buf, int len) {
        while (len > 0) {
            const uint32_t val = Random();
            memcpy(buf, &val, min<size_t>(sizeof val, len));
            len -= 4;
            buf += 4;
        }
    }
};

#endif // __STM32_RANDOM_H__
