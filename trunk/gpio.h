#ifndef __GPIO_H__
#define __GPIO_H__


enum {
	IOPIN = 0,
	IOSET = 1,
	IODIR = 2,
	IOCLR = 3
};


extern class Gpio _gpio[2];

class Gpio {
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
		_base[IODIR] &= ~(1 << pin);
#ifdef DEBUG
		_inputs |= 1 << pin;
		_outputs &= ~(1 << pin);
#endif
	}
	void MakeInputs(uint32_t mask) {
		_base[IODIR] &= ~mask;
#ifdef DEBUG
		_inputs |= mask;
		_outputs &= ~mask;
#endif
	}
	void MakeOutput(uint8_t pin) {
		_base[IODIR] |= 1 << pin;
#ifdef DEBUG
		_outputs |= 1 << pin;
		_inputs &= ~(1 << pin);
#endif
	}
	void MakeOutputs(uint32_t mask) {
		_base[IODIR] |= mask;
#ifdef DEBUG
		_outputs |= mask;
		_inputs &= ~mask;
#endif
	}
	void SetPin(uint8_t pin) {
#ifdef DEBUG
		assert(_outputs & (1 << pin));
#endif
		_base[IOSET] |= 1 << pin;
	}
	void ResetPin(uint8_t pin) {
#ifdef DEBUG
		assert(_outputs & (1 << pin));
#endif
		_base[IOCLR] |= 1 << pin;
	}
	bool GetPin(uint8_t pin) {
#ifdef DEBUG
		assert(_inputs & (1 << pin));
#endif
		return (_base[IOPIN] & (1 << pin)) != 0;
	}

	// Self-contained Pin
	class NOVTABLE Pin {
		uint8_t _portnum;
		uint8_t _pin;
	public:
		Pin(uint8_t port, uint8_t pin) : _portnum(port), _pin(pin) { }
		Pin(const Pin& arg) : _portnum(arg._portnum), _pin(arg._pin) { }
		Pin& operator=(const Pin& arg) {
			if (&arg != this) { new (this) Pin(arg); }
			return *this;
		}

		void Set() { _gpio[_portnum].SetPin(_pin); }
		void Reset() { _gpio[_portnum].ResetPin(_pin); }
		bool Get() { return _gpio[_portnum].GetPin(_pin); }
	};

	const Pin GetPin(uint8_t pin) const { return Pin(_portnum, pin); }
};


class Output {
public:
	virtual void Raise() = 0;
	virtual void Lower() = 0;
//	virtual ~Output() { }
};


class PinOutput: public Gpio::Pin {
public:
	PinOutput(const Gpio::Pin& pin) : Gpio::Pin(pin) { }
	void Raise() { Set(); }
	void Lower() { Reset(); }
private:
	PinOutput();				// Disabled
};


class PinNegOutput: public Gpio::Pin {
public:
	PinNegOutput(const Gpio::Pin& pin) : Gpio::Pin(pin) { }
	void Raise() { Reset(); }
	void Lower() { Set(); }
private:
	PinNegOutput();				// Disabled
};


#endif // __GPIO_H__
