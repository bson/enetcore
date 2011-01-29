#ifndef __SDCARD_H__
#define __SDCARD_H__

#include "mutex.h"
#include "blockdev.h"


class SDCard: public BlockDev {
	Mutex _lock;
	SpiDev& _spi;
	Output* _drivelock;			// Removal lock (or warning light)
	uint8_t _inuse;				// Use counter
	bool _initialized:1;
	bool _version2:1;
	bool _sdhc:1;				// High capacity

public:
	SDCard(SpiDev& spi);

	// Try initializing SD card.
	// Will reset the card into SPI mode.
	// Returns true if card was found and it could be initialized.
	bool Init();                // * implements BlockDev::Init()
	
	// Specify device lock
	void SetLock(Output* lock);

	uint GetSectorSize() const { return 512; } // * implements BlockDev::GetSectorSize()
	bool ReadSector(uint secnum, void* buf);   // * implements BlockDev::ReadSector()
    // * implements BlockDev::WriteSector() - NYI
	bool WriteSector(uint secnum, const void* buf) { abort(); }

	// Prevent media removal
	void Retain();              // * implements BlockDev::Retain()
	void Release();             // * implements BlockDev::Release()

private:
	// Send SD CMD
	uint8_t SendCMD(uint8_t cmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);
	uint8_t SendACMD(uint8_t acmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);

	// Send SD CMD, expecting num bytes back (e.g. 5 bytes for an R4 response)
	uint64_t SendCMDR(uint8_t num, uint8_t cmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);
};


extern SDCard _sd;


#endif // __SDCARD_H__
