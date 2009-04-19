#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "spi.h"
#include "mutex.h"
#include "blockdev.h"


class SDCard: public BlockDev {
	Mutex _lock;
	SPI& _spi;
	bool _initialized:1;
	bool _version2:1;
	bool _sdhc:1;				// High capacity

public:
	SDCard(SPI& spi);

	// Try initializing card, if any
	bool Init();
	
	uint GetSectorSize() const { return 512; }
	bool ReadSector(uint secnum, void* buf);
	bool WriteSector(uint secnum, const void* buf) { abort(); }

private:
	// Send SD CMD
	uint8_t SendCMD(uint8_t cmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);
	uint8_t SendACMD(uint8_t acmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);

	// Send SD CMD, expecting num bytes back (e.g. 5 bytes for an R4 response)
	uint64_t SendCMDR(uint8_t num, uint8_t cmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);
};


extern SDCard _sd;


#endif // __SDCARD_H__
