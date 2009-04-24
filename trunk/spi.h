#ifndef __SPI_H__
#define __SPI_H__

#include "mutex.h"
#include "gpio.h"
#include "crc16.h"


class SPI {
private:
	volatile uint8_t* _base;
	Output* _ssel;				// SSEL output or NULL if none
	uint8_t _prescaler;

public:

	SPI(uint32_t base);

	void Init();
	INLINE_ALWAYS void SetSSEL(Output* ssel) { _ssel = ssel; }

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

	// Read a given number of bytes, appending to buffer, computing CRC on the fly.
	// Returns false if we had an error during the receive
	bool ReadBuffer(void* buffer, uint len, Crc16* crc = NULL);
};

extern SPI _spi0, _spi1;

#endif // __SPI_H__
