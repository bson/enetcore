#ifndef __STM32_SYSCFG_H__
#define __STM32_SYSCFG_H__

#include "core/bits.h"

#error not yet updated for STM32H7

class Stm32SysCfg {
    enum class Register {
        MEMRMP  = 0x00,
        PMC     = 0x04,
        EXTICR1 = 0x08,
        CMPCR = 0x20
    };

public:
    enum {
        // REMAP, reset = boot pins for mode
        MEM_MODE = 0,

        // PMC
        MII_RMII_SEL = 23,

        // CMPCR
        READY = 8,
        CMP_PD = 0
    };

    enum Port {
        A = 0, B, C, D, E, F, G, H, I, END
    };
        

private:
    static volatile uint32_t& reg(const Register r) {
        return *((volatile uint32_t*)(BASE_SYSCFG + (uint32_t)r));
    }

public:
    static void ConfigureExti(Port p, uint32_t pin) {
        pin &= 0xf;
        volatile uint32_t& r = reg(Register((uint32_t)Register::EXTICR1 + pin/4));
        r = (r & ~(0b1111 << ((pin % 4) * 4)))
            | ((uint32_t)p << ((pin % 4) * 4));
    }

    static void Init(bool cmpcell) {
        if (cmpcell) {
            volatile uint32_t& r = reg(Register::CMPCR);
            r |= BIT(CMP_PD);
            while (!(r & BIT(READY)))
                ;
        }
    }

    // Unique device ID.  It's 96 bits, but we present it as two separate IDs,
    // one 32-bit and one 64-bit.
    [[__finline]] static uint32_t UniqueID32() { return *(uint32_t*)0x1fff7a10; }
    [[__finline]] static uint64_t UniqueID64() { return *(uint32_t*)0x1fff7a14; }
};

#endif // __STM32_H__
