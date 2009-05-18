#ifndef __SPI_H__
#define __SPI_H__

#include "mutex.h"
#include "gpio.h"
#include "crc16.h"


// SPI bus
class SpiBus {
	volatile uint8_t* _base;
	uint8_t _prescaler;			// Current prescaler setting

public:

	SpiBus(uint32_t base);

	void Init();

	// Recomputes prescaler
	void SetSpeed(uint hz);

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

extern SpiBus _spi0, _spi1;


// Simple SPI device
// Devices are associated with a bus and are distinguished by the output
// used for SSEL.
// While each device has its own speed, currently polarity and clock phase
// aren't settable per device.

class SpiDev {
	SpiBus& _bus;
	Output* _ssel;				// SSEL output for this device or NULL if none
	uint _speed;				// Speed setting
	bool _selected;				// Tracks whether currently selected
public:
	SpiDev(SpiBus& bus);
	
	// Init is currently a no-op
	INLINE_ALWAYS void Init() { }
	INLINE_ALWAYS void SetSSEL(Output* ssel) { _ssel = ssel; }
	void SetSpeed(uint hz);

	void Select();
	void Deselect();

	// These are delegated from bus - see SPI declaration for comments
	INLINE_ALWAYS uint8_t Send(const uint8_t* s, uint len) { return _bus.Send(s, len); }
	INLINE_ALWAYS uint8_t Read(uint8_t code = 0xff) { return _bus.Read(code); }
	INLINE_ALWAYS uint8_t ReadReply(uint interval, uint num_tries, uint8_t code = 0xff) {
		return _bus.ReadReply(interval, num_tries, code);
	}
	INLINE_ALWAYS bool ReadBuffer(void* buffer, uint len, Crc16* crc = NULL) {
		return _bus.ReadBuffer(buffer, len, crc);
	}
};


#endif // __SPI_H__
