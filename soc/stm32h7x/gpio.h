// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_GPIO_H__
#define __STM32_GPIO_H__

#include "core/bits.h"


class Stm32Gpio {
    enum class Register {
        GPIO_MODER    = 0x00,
        GPIO_OTYPER   = 0x04,
        GPIO_OSPEEDER = 0x08,
        GPIO_PUPDR    = 0x0c,
        GPIO_IDR      = 0x10,
        GPIO_ODR      = 0x14,
        GPIO_BSRR     = 0x18,
        GPIO_LCKR     = 0x1c,
        GPIO_AFRL     = 0x20,
        GPIO_AFRH     = 0x24,
    };

public:
    enum class Port {
        A = 0, B, C, D, E, F, G, H, I, J, K, END
    };

    enum class Mode {
        IN        = 0,
        OUT       = 1,
        AF        = 2,
        ANALOG    = 3
    };

    enum class Type {
        NONE      = 0b0000,
        FLOAT     = 0b0000,
        PUR       = 0b0001,
        PDR       = 0b0010,
        OD_PUR    = 0b0101,
        OD_PDR    = 0b0110
    };

    enum class Speed {
        LOW    = 0,
        MEDIUM = 1,
        FAST   = 2,
        HIGH   = 3
    };

#define PINCONF(PORT, PIN, MODE, AF, TYPE, SPEED) \
    { Stm32Gpio::Port::PORT, PIN, Stm32Gpio::Mode::MODE, AF, Stm32Gpio::Type::TYPE, Stm32Gpio::Speed::SPEED }

    struct PinConf {
        Port     port;
        uint8_t  pin:5;  // 0-32
        Mode     mode;
        uint8_t  af:4;   // 0-15
        Type     type;
        Speed    speed;
    };

    template <typename T>
    static T& reg(const Port p, const Register r) {
        return *((T*)(BASE_GPIOA + (uint32_t)p*0x400 + (uint32_t)r)); 
    }

    static void PortConfig(const PinConf* pinconf) {
        while (pinconf->port != Port::END) {
            volatile uint32_t& mode = reg<volatile uint32_t>(pinconf->port, Register::GPIO_MODER);
            mode = (mode & ~(3 << (pinconf->pin*2)))
                | ((uint32_t)pinconf->mode << (pinconf->pin*2));

            if (pinconf->mode == Mode::AF) {
                volatile uint32_t& af = reg<volatile uint32_t>(pinconf->port,
                                   pinconf->pin < 8 ? Register::GPIO_AFRL : Register::GPIO_AFRH);
                const uint32_t shift = (pinconf->pin & 7) * 4;
                af = (af & ~(0xf << shift)) 
                    | (pinconf->af << shift);
            }

            volatile uint32_t& speed = reg<volatile uint32_t>(pinconf->port, Register::GPIO_OSPEEDER);
            speed = (speed & ~(3 << (pinconf->pin*2)))
                | ((uint32_t)pinconf->speed << (pinconf->pin*2));

            volatile uint32_t& otyper = reg<volatile uint32_t>(pinconf->port, Register::GPIO_OTYPER);
            otyper |= (otyper & ~BIT(pinconf->pin))
                | (pinconf->type >= Type::OD_PUR ? BIT(pinconf->pin) : 0);

            volatile uint32_t& pupdr = reg<volatile uint32_t>(pinconf->port, Register::GPIO_PUPDR);
            pupdr = (pupdr & ~(3 << pinconf->pin*2))
                | (((uint32_t)pinconf->type & 0b0011) << (pinconf->pin*2));

            ++pinconf;
        }
    }
    
    [[__finline]] static void Set(Port port, uint32_t mask) {
        reg<volatile uint32_t>(port, Register::GPIO_BSRR) = mask;
    }

    [[__finline]] static void Clear(Port port, uint32_t mask) {
        reg<volatile uint32_t>(port, Register::GPIO_BSRR) = mask << 16;
    }

    [[__finline]] static uint32_t Input(Port port) {
        return reg<volatile uint32_t>(port, Register::GPIO_IDR);
    }

    [[__finline]] static void Output(Port port, uint32_t mask, uint32_t value) {
        volatile uint32_t& r = reg<volatile uint32_t>(port, Register::GPIO_ODR);
        r = (r & ~mask) | value;
    }

    static void SetPin(Port port, uint8_t pin) {
        reg<volatile uint32_t>(port, Register::GPIO_BSRR) = 1 << pin;
    }

    static void ResetPin(Port port, uint8_t pin) {
        reg<volatile uint32_t>(port, Register::GPIO_BSRR) = (1 << 16) << pin;
    }

    static bool TestPin(Port port, uint8_t pin) {
        return reg<volatile uint32_t>(port, Register::GPIO_IDR) & (1 << pin);
    }
};


class Stm32GpioPort {
public:
    typedef Stm32Gpio::Port Port;

private:
    Port _port;

public:
    Stm32GpioPort(Port port) : _port(port) { }
    
    [[__finline]] void Set(uint32_t mask) { Stm32Gpio::Set(_port, mask); }
    [[__finline]] void Clear(uint32_t mask) { Stm32Gpio::Clear(_port, mask); }
    [[__finline]] uint32_t Input() { return Stm32Gpio::Input(_port); }
    [[__finline]] void Output(uint32_t mask, uint32_t value) { Stm32Gpio::Output(_port, mask, value); }

    // Self-contained Pin
	class __novtable Pin {
        Port    _port;
		uint8_t _pin:4;
		
	public:
        Pin() : _port(Port::A), _pin(0) { }
		Pin(Port port, uint8_t pin) : _port(port), _pin(pin) { }
		Pin(const Pin& arg) : _port(arg._port), _pin(arg._pin) { }
		Pin& operator=(const Pin& arg) {
			if (&arg != this) {
                new (this) Pin(arg);
            }
			return *this;
		}

		void Set() { Stm32Gpio::SetPin(_port, _pin); }
		void Reset() { Stm32Gpio::ResetPin(_port, _pin); }
		bool Test() { return Stm32Gpio::TestPin(_port, _pin); }

		void operator=(uint arg) { if (arg) Set(); else Reset(); }
		operator bool() { return Test(); }
	};

	Pin GetPin(uint8_t pin) { return Pin(_port, pin); }
};

#endif // __STM32_GPIO_H__
