#ifndef __STM32_SYSCFG_H__
#define __STM32_SYSCFG_H__

class Stm32SysCfg {
    enum Register {
        SYSCFG_MEMRMP  = 0x00,
        SYSCFG_PMC     = 0x04,
        SYSCFG_EXTICR1 = 0x08,
        SYSCFG_CMPCR = 0x20
    };

public:
    enum {
        // SYSCFG_REMAP, reset = boot pins for mode
        MEM_MODE = 0,

        // SYSCFG_PMC
        MII_RMII_SEL = 23,

        // SYSCFG_CMPCR
        READY = 8,
        CMP_PD = 0
    };

    enum Port {
        A = 0, B, C, D, E, F, G, H, I
    };
        

private:
    template typename <T>
    [[_finline]] static T& reg(const Register r) { return *((T*)(BASE_SYSCFG + (uint32_t)r)); }

public:
    static void MapExti(Port p, uint32_t pin) {
        pin &= 0xf;
        volatile uint32_t& r = reg<volatile uint32_t>(SYSCFG_EXTICR1 + pin/4);
        r = (r & ~(0b1111 << ((pin % 4) * 4)))
            | ((uint32_t)p << ((pin % 4) * 4));
    }

    static void Init(bool cmpcell) {
        if (cmpcell) {
            volatile uint32_t& r = reg<volatile uint32_t>(SYSCFG_CMPCR);
            r |= BIT(CMP_PD);
            while (!(r & BIT(READY)))
                ;
    }
};

#endif // __STM32_SYSCFG_H__
