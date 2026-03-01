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
    struct Capability {
        uint32_t  max_default_freq;   /* Hz (from CSD) */
        uint8_t   high_speed_supported;
        uint8_t   uhs_supported;
        uint8_t   sd_spec;            /* 1, 2, 3, etc */
        uint8_t   bus_4bit_supported;
    };

    Sdio&      _sdio;           // SDIO 1 & 4 bit compatible interface
    uint32_t   _rca;            // Relative Card Address
    uint32_t   _size;           // Sector count
    Capability _caps;           // Speed capabilities
    bool       _high_capacity;  // SDHC/SDXC
    uint8_t    _csd[16];        // Card CSD

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
    // Fetch card CSD
    int read_csd();

    // Examine CSD determine card size
    int find_size();

    // Determine default speed
    uint32_t parse_tran_speed(uint8_t ts);

    // Read SCR
    int read_scr(uint8_t scr[8]);

    // Parse SCR to obtain SD spec and bus widths
    void parse_scr(const uint8_t scr[8]);

    // Check for HS support
    bool check_high_speed();

    // Obtain speed capabilities
    int get_speed_caps();

    int wait_ready();
};

#endif // __SDCARD_H__
