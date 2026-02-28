//
// Copyright 2026 Jan Brittenson
// See LICENSE for details. 
//

#ifndef __SDCARD_H__
#define __SDCARD_H__

#include <stdint.h>
#include "core/sdio.h"
#include "core/blockdev.h"


class SDCard: public BlockDev {

    Sdio&     _sdio;            // SDIO 1 & 4 bit compatible interface
    uint32_t  _rca;             // Relative Card Address
    uint32_t  _size;            // Sector count
    bool      _high_capacity;   // SDHC/SDXC

    enum : uint16_t { BLOCK_SIZE = 512 };


public:
    SDCard(Sdio& sdio)
        :  _sdio(sdio),
           _rca(0),
           _high_capacity(false)
    {
    }

    // BlockDev interface
    int init();
    int read_blocks(uint32_t lba, uint32_t count, void *buffer, bool);
    int write_blocks(uint32_t lba, uint32_t count, const void *buffer, bool);
    int flush() { return 0; }
    uint32_t sector_size() const { return BLOCK_SIZE; }
    uint32_t size() const { return _size; };

private:
    int read_csd();
    int wait_ready();
};

#endif // __SDCARD_H__
