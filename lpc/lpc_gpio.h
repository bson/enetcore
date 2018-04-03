// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __LPC_GPIO_H__
#define __LPC_GPIO_H__


// GPIO and Output abstraction
//
//
// Each GPIO port (or device, if you prefer) has an instance of this
// class for it.  The port object can then return Pin objects, which
// are simple proxies to perform operations.
//
// An Output is an abstraction that's basically used to turn things on
// and off.  A typical example might be a LED, relay control, or a
// peripheral device select.  But it could also be a pure software
// control to, for example, signal an event object.  Its purpose is to
// allow parameterization of control signals.  A PinOutput is an
// implementation of Output that transmits the Output control to a
// GPIO pin.  PinNegOutput is the same, except opposite polarity.

class LpcGpio {
	volatile uint32_t* _base;

    enum {
        REG_DIR   = 0x00,
        REG_MASK  = 0x10/4,
        REG_PIN   = 0x14/4,
        REG_SET   = 0x18/4,
        REG_CLR   = 0x1c/4
    };

public:
	LpcGpio(uintptr_t base) 
		: _base((volatile uint32_t*)base)
	{ }

    // Mask pins
    void Mask(uint32_t mask) {
        _base[REG_MASK] |= ~mask;
    }

	void MakeInput(uint8_t pin) {
		_base[REG_DIR]  &= ~(1 << pin);
        _base[REG_MASK] &= ~(1 << pin);
	}
	void MakeInputs(uint32_t mask) {
		_base[REG_DIR]  &= ~mask;
        _base[REG_MASK] &= ~mask;
	}
	void MakeOutput(uint8_t pin) {
		_base[REG_DIR]  |= 1 << pin;
        _base[REG_MASK] &= ~(1 << pin);
	}
	void MakeOutputs(uint32_t mask) {
		_base[REG_DIR]  |= mask;
        _base[REG_MASK] &= ~mask;
	}
    [[__finline]] void Set(uint32_t mask) {
        _base[REG_SET] = mask;
    }
    [[__finline]] void Clear(uint32_t mask) {
        _base[REG_CLR] = mask;
    }
    [[__finline]] uint32_t Input() { 
        return _base[REG_PIN]; 
    }
    [[__finline]] void Output(uint32_t mask, uint32_t value) {
        _base[REG_PIN] = (_base[REG_PIN] & ~mask) | value;
    }
	void SetPin(uint8_t pin) {
		_base[REG_SET] = 1 << pin;
	}
	void ResetPin(uint8_t pin) {
		_base[REG_CLR] = 1 << pin;
	}
	bool TestPin(uint8_t pin) const {
		return (_base[REG_PIN] & (1 << pin)) != 0;
	}

    // Get addresses of registers
    uintptr_t RegPin() const { return (uintptr_t)(_base + REG_PIN); }
    uintptr_t RegSet() const { return (uintptr_t)(_base + REG_SET); }
    uintptr_t RegClr() const { return (uintptr_t)(_base + REG_CLR); }

	// Self-contained Pin
	class __novtable Pin {
        class LpcGpio& _gpio;
		uint8_t _pin;
		
	public:
        Pin() : _gpio(*(class LpcGpio*)0), _pin(0) { }
		Pin(class LpcGpio& gpio, uint8_t pin) : _gpio(gpio), _pin(pin) { }
		Pin(const Pin& arg) : _gpio(arg._gpio), _pin(arg._pin) { }
		Pin& operator=(const Pin& arg) {
			if (&arg != this) { new (this) Pin(arg); }
			return *this;
		}

		void Set() { _gpio.SetPin(_pin); }
		void Reset() { _gpio.ResetPin(_pin); }
		bool Test() const {
			return _gpio.TestPin(_pin);
		}

		void operator=(uint arg) { if (arg) Set(); else Reset(); }
		operator bool() const { return Test(); }
	};

	Pin GetPin(uint8_t pin) { return Pin(*this, pin); }
};


#endif // __LPC_GPIO_H__
