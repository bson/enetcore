#ifndef __SPI_H__
#define __SPI_H__

#include "mutex.h"


#define SPI0_BASE 0xe0020000
// #define SPI1_BASE 0xe0030000

enum {
	SPCR = 0,
	SPSR = 4,
	SPDR = 8,
	SPCCR = 0xc,
	SPINT = 0x1c
};

class SPI {
private:
	volatile uint8_t* _base;

public:

	SPI(uint32_t base);

	void Init();
	void SetSpeed(uint hz);

	void Select();
	void Deselect();

	// Send byte sequence, returns byte received on last byte
	uint8_t Send(const uint8_t* s, uint len);

	// Send byte, read reply
	uint8_t Read(uint8_t code = 0xff);

	// Send byte, repeating at interval, a given number of times until
	// non-0xff is received; if so, return it.  Returns 0xff if nothing
	// was received.
	uint8_t ReadReply(uint interval, uint num_tries, uint8_t code = 0xff);

	// Read a given number of bytes, appending to buffer
	void ReadBuffer(void* buffer, uint len);

	// Keep reading response until we get give value
	bool WaitReady(uint interval, uint num_tries, uint8_t value);
};

extern SPI _spi0;

#endif // __SPI_H__
