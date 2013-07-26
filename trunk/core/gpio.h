#ifndef __GPIO_H__
#define __GPIO_H__


class Output {
public:
	virtual void Raise() = 0;
	virtual void Lower() = 0;
//	virtual ~Output() { }
};


template <typename T>
class PinOutput: public T,
				 public Output {
public:
	PinOutput() { }
	PinOutput(const T& pin) : T(pin) { }
	PinOutput(const PinOutput& arg) : T(arg) { }
	PinOutput& operator=(const PinOutput& arg) { new (this) PinOutput(arg); return *this; }
	void Raise() { T::Set(); }
	void Lower() { T::Reset(); }
};


template <typename T>
class PinNegOutput: public T,
					public Output {
public:
	PinNegOutput() { }
	PinNegOutput(const T& pin) : T(pin) { }
	PinNegOutput(const PinNegOutput& arg) : T(arg) { }
	PinNegOutput& operator=(const PinNegOutput& arg) {
		new (this) PinNegOutput(arg); return *this;
	}
	void Raise() { T::Reset(); }
	void Lower() { T::Set(); }
};

#endif // __GPIO_H__
