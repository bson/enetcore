#ifndef __EINTR_H__
#define __EINTR_H__

#include "soc/stm32f4x/syscfg.h"
#include "core/bits.h"


class Stm32Eintr {
    enum class Register {
        IMR = 0x00,
        EMR = 0x04,
        RTSR = 0x08,
        FTSR = 0x0c,
        SWIER = 0x10,
        PR = 0x14
    };

    static volatile uint32_t& reg(Register r) { 
        return *(volatile uint32_t*)(BASE_EXTI + (uint32_t)r);
    }

public:
    // We want these to be the same
    typedef Stm32SysCfg::Port Port;

    enum class Trigger {
        RISING = 0,
        FALLING = 1,
        BOTH = 2
    };


#define EINTRCONF(PORT, PIN, TRIGGER) \
    { Stm32Eintr::Port::PORT, (PIN), Stm32Eintr::Trigger::TRIGGER }

    struct EintrConf {
        Port port;
        uint8_t pin:4;
        Trigger trigger;
    };

    static void EintrConfig(const EintrConf* config) {
#ifdef DEBUG
        uint16_t pins_used = 0;
#endif
        while (config->port != Port::END) {
            const uint32_t bit = BIT(config->pin);
#ifdef DEBUG
            assert((pins_used & bit) == 0);
            pins_used |= bit;
#endif
            Stm32SysCfg::ConfigureExti(config->port, config->pin);
            reg(Register::IMR) &= ~bit;
            reg(Register::EMR) &= ~bit;

            if (config->trigger == Trigger::RISING || config->trigger == Trigger::BOTH)
                reg(Register::RTSR) |= bit;

            if (config->trigger == Trigger::FALLING || config->trigger == Trigger::BOTH)
                reg(Register::FTSR) |= bit;

            reg(Register::PR) &= ~bit;
            reg(Register::IMR) |= bit;
            ++config;
        }
    }

    static void EnableInt(uint32_t pin) {
        reg(Register::IMR) |= BIT(pin);
    }

    static void DisableInt(uint32_t pin) {
        reg(Register::IMR) &= ~BIT(pin);
    }

    static bool Pending(uint32_t pin) {
        return reg(Register::PR) & BIT(pin);
    }

    static void ClearPending(uint32_t pin) {
        reg(Register::PR) |= BIT(pin); // Bit is cleared by writing a '1' to it (!!!)
    }

    static void PostInterrupt(uint32_t pin) {
        reg(Register::SWIER) |= BIT(pin);
    }
};

#endif // __EINTR_H__
