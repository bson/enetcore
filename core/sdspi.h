// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __SDSPI_H__
#define __SDSPI_H__

#include "core/mutex.h"
#include "core/blockdev.h"

// RO SPI implementation

class SDSpi: public BlockDev {
	Mutex   _lock;
	SpiDev& _spi;
	Output* _drivelock;			// Removal lock (or warning light)
	uint8_t _inuse;				// Use counter
	bool    _initialized:1;
	bool    _version2:1;
	bool    _sdhc:1;            // High capacity

    enum {
        CMD0_TIMEOUT = 10000,   // How long we wait for CMD0 (usec)
        CMD_TIMEOUT = 100,      // Limit for how long we wait for a command response (usec)
        INIT_TIMEOUT = 1500,    // Initialization timeout (msec)
        SECTOR_SIZE= 512,
    };

public:
	SDSpi(SpiDev& spi);

	// Try initializing SD card.
	// Will reset the card into SPI mode.
	// Returns true if card was found and it could be initialized.
	bool init();                // * implements BlockDev::Init()
	
	// Specify device lock
	void SetLock(Output* lock);

    // * implements BlockDev::sector_size()
	uint32_t sector_size() const { return SECTOR_SIZE; }

    // * implements BlockDev::size()
    uint32_t size() const { return 0 };

    // * implements BlockDev::ReadSector()
	int read_blocks(uint32_t secnum, uint32_t count, void* buf, bool bypass);

    // * implements BlockDev::WriteSector() - NYI
	int write_blocks(uint secnum, uint32_t count, const void* buf. bool bypass) {
        abort(); return -1;
    }

	// Prevent media removal
	void retain();              // * implements BlockDev::Retain()
	void release();             // * implements BlockDev::Release()

private:
	// Send SD CMD
	int SendCMD(uint8_t cmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0, bool ssel = true);
	int SendACMD(uint8_t acmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);

	// Send SD CMD, expecting num bytes back (e.g. 5 bytes for an R4 response)
	uint64_t SendCMDR(uint8_t num, uint8_t cmd, uint16_t a = 0, uint8_t b = 0, uint8_t c = 0);
};


#ifdef ENABLE_SDSPI
extern SDSpi _sd;
#endif

#endif // __SDSPI_H__
