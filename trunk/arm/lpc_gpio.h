#ifndef __LPC_GPIO_H__
#define __LPC_GPIO_H__


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


class Output {
public:
	virtual void Raise() = 0;
	virtual void Lower() = 0;
//	virtual ~Output() { }
};


class PinOutput: public LpcGpio::Pin,
				 public Output {
public:
	PinOutput() { }
	PinOutput(const LpcGpio::Pin& pin) : LpcGpio::Pin(pin) { }
	PinOutput(const PinOutput& arg) : LpcGpio::Pin(arg) { }
	PinOutput& operator=(const PinOutput& arg) { new (this) PinOutput(arg); return *this; }
	void Raise() { Set(); }
	void Lower() { Reset(); }
};


class PinNegOutput: public LpcGpio::Pin,
					public Output {
public:
	PinNegOutput() { }
	PinNegOutput(const LpcGpio::Pin& pin) : LpcGpio::Pin(pin) { }
	PinNegOutput(const PinNegOutput& arg) : LpcGpio::Pin(arg) { }
	PinNegOutput& operator=(const PinNegOutput& arg) {
		new (this) PinNegOutput(arg); return *this;
	}
	void Raise() { Reset(); }
	void Lower() { Set(); }
};


#endif // __LPC_GPIO_H__
