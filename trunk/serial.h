#ifndef __SERIAL_H__
#define __SERIAL_H__


class SerialPort {
	volatile uint8_t* _base;

public:
	SerialPort(volatile void* base, uint default_speed = 9600) : 
		_base((volatile uint8_t*)base)
	{
		SetSpeed(9600);
	}

	~SerialPort() { }

	void SetSpeed(uint speed);

	// Send synchronously (= polled) 
	void WriteSync(const String& s);
private:
	void WriteSync(const uchar* buf, uint len);
};


extern SerialPort _console;
extern SerialPort _lcd;

#endif // __SERIAL_H__
