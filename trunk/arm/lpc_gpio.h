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
//
// XXX the simple Output base class should go somewhere generic


class LpcGpio {
	volatile uint32_t* _base;
	uint8_t _portnum;
#ifdef DEBUG
	// Track inputs and outputs
	uint32_t _inputs;
	uint32_t _outputs;
#endif
public:
	void Init(uint32_t base, uint8_t port) {
		_base = (volatile uint32_t*)base;
		_portnum = port;
#ifdef DEBUG
		_inputs = _outputs = 0;
#endif
	}
	void MakeInput(uint8_t pin) {
		_base[GPIO_IODIR] &= ~(1 << pin);
#ifdef DEBUG
		_inputs |= 1 << pin;
		_outputs &= ~(1 << pin);
#endif
	}
	void MakeInputs(uint32_t mask) {
		_base[GPIO_IODIR] &= ~mask;
#ifdef DEBUG
		_inputs |= mask;
		_outputs &= ~mask;
#endif
	}
	void MakeOutput(uint8_t pin) {
		_base[GPIO_IODIR] |= 1 << pin;
#ifdef DEBUG
		_outputs |= 1 << pin;
		_inputs &= ~(1 << pin);
#endif
	}
	void MakeOutputs(uint32_t mask) {
		_base[GPIO_IODIR] |= mask;
#ifdef DEBUG
		_outputs |= mask;
		_inputs &= ~mask;
#endif
	}
	void SetPin(uint8_t pin) {
#ifdef DEBUG
		assert(_outputs & (1 << pin));
#endif
		_base[GPIO_IOSET] |= 1 << pin;
	}
	void ResetPin(uint8_t pin) {
#ifdef DEBUG
		assert(_outputs & (1 << pin));
#endif
		_base[GPIO_IOCLR] |= 1 << pin;
	}
	bool TestPin(uint8_t pin) const {
#ifdef DEBUG
		assert(_inputs & (1 << pin));
#endif
		return (_base[GPIO_IOPIN] & (1 << pin)) != 0;
	}

	// Self-contained Pin
	class __novtable Pin {
		uint8_t _portnum;
		uint8_t _pin;
		
	public:
		Pin() : _portnum(0), _pin(0) { }
		Pin(uint8_t port, uint8_t pin) : _portnum(port), _pin(pin) { }
		Pin(const Pin& arg) : _portnum(arg._portnum), _pin(arg._pin) { }
		Pin& operator=(const Pin& arg) {
			if (&arg != this) { new (this) Pin(arg); }
			return *this;
		}

		void Set() { extern class LpcGpio _gpio[]; _gpio[_portnum].SetPin(_pin); }
		void Reset() { extern class LpcGpio _gpio[]; _gpio[_portnum].ResetPin(_pin); }
		bool Test() const {
			extern class LpcGpio _gpio[]; 
			return _gpio[_portnum].TestPin(_pin);
		}

		void operator=(uint arg) { if (arg) Set(); else Reset(); }
		operator bool() const { return Test(); }
	};

	const Pin GetPin(uint8_t pin) const { return Pin(_portnum, pin); }
};


#endif // __LPC_GPIO_H__
