// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __EINTR_H__
#define __EINTR_H__

#include "soc/stm32h7x/syscfg.h"
#include "core/bits.h"

class Stm32Eintr {
    enum class Register {
        RTSR1   = 0x00,
        FTSR1   = 0x04,
        SWIER1  = 0x08,
        CPUIMR1 = 0x80,         // EXTI are in the low 15 bits of CPUIMR1
        CPUEMR1 = 0x84,
        CPUPR1  = 0x88,
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
            reg(Register::CPUIMR1) &= ~bit;
            reg(Register::CPUEMR1) &= ~bit;

            if (config->trigger == Trigger::RISING || config->trigger == Trigger::BOTH)
                reg(Register::RTSR1) |= bit;

            if (config->trigger == Trigger::FALLING || config->trigger == Trigger::BOTH)
                reg(Register::FTSR1) |= bit;

            reg(Register::CPUPR1) &= ~bit;
            reg(Register::CPUIMR1) |= bit;
            ++config;
        }
    }

    static void EnableInt(uint32_t pin) {
        reg(Register::CPUIMR1) |= BIT(pin);
    }

    static void DisableInt(uint32_t pin) {
        reg(Register::CPUIMR1) &= ~BIT(pin);
    }

    static bool Pending(uint32_t pin) {
        return reg(Register::CPUPR1) & BIT(pin);
    }

    static void ClearPending(uint32_t pin) {
        reg(Register::CPUPR1) |= BIT(pin); // Bit is cleared by writing a '1' to it (!!!)
    }

    static void PostInterrupt(uint32_t pin) {
        reg(Register::SWIER1) |= BIT(pin);
    }
};

#endif // __EINTR_H__
