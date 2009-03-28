#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "spi.h"
#include "mutex.h"

class SDCard {
	Mutex _lock;
	SPI& _spi;
	bool _initialized;
	bool _version2;

public:
	SDCard(SPI& spi);
	void Init();
	
private:
	// Send SD CMD
	uint8_t SendCMD(uint8_t cmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);
	uint8_t SendACMD(uint8_t acmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);
};


extern SDCard _sd;


#endif // __SDCARD_H__
