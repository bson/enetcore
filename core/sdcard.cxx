//
// Copyright 2026 Jan Brittenson
// See LICENSE for details. 
//

#include <stdint.h>
#include "core/enetcore.h"
#include "core/sdcard.h"
#include "core/sdio.h"
#include "core/bits.h"


int SDCard::read_csd()
{
    uint32_t resp[4];

    if (_sdio.send_cmd(9, _rca, Sdio::SD_RESP_R2, resp))
        return -1;

    /* resp[] holds 128-bit CSD */

    for (int i = 0; i < 4; i++) {
        _csd[i*4+0] = resp[3-i] >> 24;
        _csd[i*4+1] = resp[3-i] >> 16;
        _csd[i*4+2] = resp[3-i] >> 8;
        _csd[i*4+3] = resp[3-i];
    }

    return 0;
}


int SDCard::find_size()
{
    const uint8_t csd_structure = (_csd[0] >> 6) & 0x3;

    if (csd_structure == 1) {
        /* SDHC/SDXC */
        const uint32_t c_size =
            ((uint32_t)(_csd[7] & 0x3f) << 16) | ((uint32_t)_csd[8] << 8) | _csd[9];

        _size = (c_size + 1) * 1024;
    } else {
        /* SDSC */
        const uint32_t c_size =
            ((uint32_t)(_csd[6] & 0x3) << 10) | ((uint32_t)_csd[7] << 2) | ((_csd[8] >> 6) & 0x3);

        const uint8_t c_size_mult =
            ((_csd[9] & 0x3) << 1) | ((_csd[10] >> 7) & 0x1);

        const uint8_t read_bl_len = _csd[5] & 0xf;
        const uint32_t block_len = BIT(read_bl_len);
        const uint32_t mult = BIT(c_size_mult + 2);
        const uint32_t blocknr = (c_size + 1) * mult;

        const uint32_t capacity_bytes = blocknr * block_len;
        _size = capacity_bytes / BLOCK_SIZE;
    }

    return 0;
}


uint32_t SDCard::parse_tran_speed(uint8_t ts)
{
    static const uint32_t unit[] = {
        10000,     /* 100 kbit/s */
        100000,    /* 1 Mbit/s */
        1000000,   /* 10 Mbit/s */
        10000000   /* 100 Mbit/s */
    };

    static const uint8_t mult[] = {
        0,10,12,13,15,20,26,30,
        35,40,45,52,55,60,70,80
    };
    
    const uint8_t u = ts & 0x7;
    const uint8_t m = (ts >> 3) & 0xf;

    if (u > 3 || m > 15)
        return 0;

    return unit[u] * mult[m] / 10;
}


int SDCard::read_scr(uint8_t scr[8])
{
    uint32_t resp;

    if (_sdio.send_cmd(55, _rca, 1, &resp))
        return -1;

    if (_sdio.send_cmd(51, 0, 1, &resp))
        return -1;

    if (_sdio.data_read(scr, 8))
        return -1;

    return 0;
}


void SDCard::parse_scr(const uint8_t scr[8])
{
    uint32_t scr0 =
        (scr[0] << 24) |
        (scr[1] << 16) |
        (scr[2] << 8)  |
        scr[3];

    _caps.sd_spec = (scr0 >> 24) & 0xf;

    const uint32_t bus_widths = (scr0 >> 16) & 0xf;
    if (bus_widths & 0x4)
        _caps.bus_4bit_supported = 1;
}


bool SDCard::check_high_speed()
{
    uint8_t status[64];

    /* CMD6 mode 0 (check) */
    const uint32_t arg =
        (0 << 31) |       /* mode 0 = check */
        (0xfffffff1);     /* request function 1 in group 1 */

    if (_sdio.send_cmd(6, arg, 1, nullptr))
        return 0;

    if (_sdio.data_read(status, 64))
        return 0;

    /* Function group 1 support bits are in byte 13 */
    if (status[13] & 0x02)
        return 1;

    return 0;
}


int SDCard::get_speed_caps()
{
    memset(&_caps, 0, sizeof(_caps));

    if (read_csd())
        return -1;

    _caps.max_default_freq = parse_tran_speed(_csd[3]);

    uint8_t scr[8];
    if (read_scr(scr))
        return -1;

    parse_scr(scr);

    _caps.high_speed_supported = check_high_speed();

    _caps.uhs_supported = (_caps.sd_spec >= 3);

    return 0;
}


int SDCard::init()
{
    uint32_t resp[4];

    _sdio.set_bus_width_1bit();
    _sdio.set_clock(400000); /* 400 kHz init */

    /* CMD0: GO_IDLE */
    if (_sdio.send_cmd(0, 0, Sdio::SD_RESP_NONE, nullptr))
        return -1;

    /* CMD8: SEND_IF_COND */
    if (_sdio.send_cmd(8, 0x1aa, Sdio::SD_RESP_R7, resp))
        return -1;

    if ((resp[0] & 0xfff) != 0x1aa)
        return -1; /* voltage mismatch */

    /* ACMD41 loop */
    for (;;) {
        /* CMD55 */
        if (_sdio.send_cmd(55, 0, Sdio::SD_RESP_R1, resp))
            return -1;

        /* ACMD41 */
        if (_sdio.send_cmd(41,
                           0x40300000, /* HCS + 3.2–3.4V */
                           Sdio::SD_RESP_R3,
                           resp))
            return -1;

        if (resp[0] & (1UL << 31))
            break; /* card ready */
    }

    if (resp[0] & (1UL << 30))
        _high_capacity = true;

    /* CMD2: ALL_SEND_CID */
    if (_sdio.send_cmd(2, 0, Sdio::SD_RESP_R2, resp))
        return -1;

    /* CMD3: SEND_REL_ADDR */
    if (_sdio.send_cmd(3, 0, Sdio::SD_RESP_R6, resp))
        return -1;

    _rca = resp[0] & 0xffff0000;

    /* CMD7: SELECT_CARD */
    if (_sdio.send_cmd(7, _rca, Sdio::SD_RESP_R1, resp))
        return -1;

    /* Set block length (not required for SDHC but harmless) */
    (void)_sdio.send_cmd(16, BLOCK_SIZE, Sdio::SD_RESP_R1, resp);

    // Determine card capabilities
    get_speed_caps();

    DMSG("\nSDMMC: size (in blocks): %u\n", _size);
    DMSG("SDMMC: SDHC/SDXC: %s\n", _high_capacity ? "yes" : "no");
    DMSG("SDMMC: default max: %u Hz\n", _caps.max_default_freq);
    DMSG("SDMMC: high speed: %s\n", _caps.high_speed_supported ? "yes" : "no");
    DMSG("SDMMC: UHS: %s\n", _caps.uhs_supported ? "yes" : "no");
    DMSG("SDMMC: 4-bit: %s\n\n", _caps.bus_4bit_supported ? "yes" : "no");

    /* Switch to 4-bit mode */
    if (_caps.bus_4bit_supported) {
        /* CMD55 */
        if (_sdio.send_cmd(55, _rca, Sdio::SD_RESP_R1, resp))
            return -1;

        /* ACMD6: set 4-bit */
        if (_sdio.send_cmd(6, 2, Sdio::SD_RESP_R1, resp))
            return -1;

        _sdio.set_bus_width_4bit();
    }

    _sdio.set_clock(25000000); /* 25 MHz normal */

    return 0;
}


int SDCard::read_blocks(uint32_t lba, uint32_t count, void *buffer, bool)
{
    if (count == 0)
        return 0;

    const uint32_t addr = _high_capacity ? lba : lba * BLOCK_SIZE;

    uint32_t resp[4];

    if (count == 1) {
        if (_sdio.send_cmd(17, addr, Sdio::SD_RESP_R1, resp))
            return -1;

        if (_sdio.data_read(buffer, BLOCK_SIZE))
            return -1;

        return _sdio.wait_data_done();
    }

    /* Multiple block read */

    if (_sdio.send_cmd(18, addr, Sdio::SD_RESP_R1, resp))
        return -1;

    if (_sdio.data_read(buffer, count * BLOCK_SIZE))
        return -1;

    if (_sdio.wait_data_done())
        return -1;

    /* CMD12: STOP_TRANSMISSION */
    if (_sdio.send_cmd(12, 0, Sdio::SD_RESP_R1, resp))
        return -1;

    return 0;
}


int SDCard::write_blocks(uint32_t lba, uint32_t count, const void *buffer, bool)
{
    if (count == 0)
        return 0;

    const uint32_t addr = _high_capacity ? lba : lba * BLOCK_SIZE;

    uint32_t resp[4];

    if (count == 1) {
        if (_sdio.send_cmd(24, addr, Sdio::SD_RESP_R1, resp))
            return -1;

        if (_sdio.data_write(buffer, BLOCK_SIZE))
            return -1;

        return _sdio.wait_data_done();
    }

    /* Multi-block write */

    if (_sdio.send_cmd(25, addr, Sdio::SD_RESP_R1, resp))
        return -1;

    if (_sdio.data_write(buffer, count * BLOCK_SIZE))
        return -1;

    if (_sdio.wait_data_done())
        return -1;

    /* STOP */
    if (_sdio.send_cmd(12, 0, Sdio::SD_RESP_R1, resp))
        return -1;

    return wait_ready();
}


int SDCard::wait_ready()
{
    uint32_t resp[4];

    for (int i = 0; i < 100000; i++) {

        if (_sdio.send_cmd(13, _rca, Sdio::SD_RESP_R1, resp))
            return -1;

        if (!(resp[0] & BIT(8))) /* READY_FOR_DATA */
            continue;

        if (((resp[0] >> 9) & 0xf) == 4) /* TRAN state */
            return 0;
    }

    return -1;
}
