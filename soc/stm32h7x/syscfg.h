#ifndef __STM32_SYSCFG_H__
#define __STM32_SYSCFG_H__

#include "core/bits.h"

class Stm32SysCfg {
    enum class Register {
        PMCR    = 0x04,         // Periph mode config reg
        EXTICR1 = 0x08,
        PKGR    = 0x124,

        // User registers
        UR0 = 0x300,
    };

public:
    enum {
        // PMCR
        EPIS = 31,

        // CMPCR
        READY = 8,
        CMP_PD = 0
    };

    enum Port {
        A = 0, B, C, D, E, F, G, H, I, END
    };
        

private:
    static volatile uint32_t& reg(const Register r) {
        return *(volatile uint32_t*)(BASE_SYSCFG + (uint32_t)r);
    }

    static volatile uint32_t& ur(int num) {
        return *(volatile uint32_t*)(BASE_SYSCFG + num * 4);
    }


public:
    static void ConfigureExti(Port p, uint32_t pin) {
        pin &= 0xf;
        volatile uint32_t& r = reg(Register((uint32_t)Register::EXTICR1 + pin/4));
        r = (r & ~(0b1111 << ((pin % 4) * 4)))
            | ((uint32_t)p << ((pin % 4) * 4));
    }

    static void Init(bool cmpcell) {
        // Set up UR
    }

    // RMII if true, otherwise MII 
    static void EthRmii(bool enable) {
        volatile uint32_t& pmcr *(volatile uint32_t*)(BASE_SYSCFG + SYSCFG_OMCR);

        pmcr = (pmcr & (~0b111 << EPIS)) | ((enable ? 0b100 : 0) << EPIS);
    }


    // Unique device ID.  It's 96 bits, but we present it as two separate IDs,
    // one 32-bit and one 64-bit.
    [[__finline]] static uint32_t UniqueID32() { return *(uint32_t*)0x1ff1e800; }
    [[__finline]] static uint64_t UniqueID64() { return *(uint32_t*)0x1ff1e804; }

    // Flash size, in kB
    [[__finline]] static uint32_t UniqueID32() { return *(uint32_t*)0x1ff1e880; }

    // Package:
    //   0 = LQFP100,
    //   2 = TQFP144,
    //   5 = TQFP176/UFBGA176,
    //   8 = LQFP208, TFBGA240
    [[__finline]] static uint8_t UniqueID32() { return *(uint8_t*)(BASE_SYSCFG + PKGR) & 0xf; }
};


#endif // __STM32_H__
