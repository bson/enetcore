// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _LPC_CRC_H_
#define _LPC_CRC_H_

#ifndef HAVE_HW_CRC

// Software substitutions
#include "crc16.h"
#include "crc32.h"

#else

#include <stdint.h>
#include "compiler.h"
#include "bits.h"

class LpcCrc {
    volatile uint32_t* _base;

    enum {
        // (RW) - CRC mode register
        REG_MODE = 0x000/4,

        // (RW) - CRC seed register
        REG_SEED =  0x004/4,

        // (RO) - CRC checksum register
        REG_SUM =  0x008/4,

        // (WO) - CRC data register
        REG_DATA =  0x008/4
    };

public:
    enum {
        MODE_POLY_CCITT = 0,
        MODE_POLY_CRC16 = 1,
        MODE_POLY_CRC32 = 2,
        MODE_BIT_RVS_WR = BIT2,
        MODE_CMPL_WR    = BIT3,
        MODE_BIT_RVS_SUM= BIT4,
        MODE_CMPL_SUM   = BIT5
    };

    LpcCrc(uintptr_t base)
        : _base((volatile uint32_t*)base) {
    }

    void SetMode(uint32_t mode) { _base[REG_MODE] = mode; }
    void SetSeed(uint32_t seed) { _base[REG_SEED] = seed; }

    // Accepts anything 8, 16, or 32 bit.
    template <typename T>
    void Update(T data) { *(volatile T*)(_base + REG_DATA) = data; }

    template <typename T>
	void Update(const T* block, uint len) {
        while (len--)
            Update(*block++);
    }

    // Get sum
    uint32_t GetValue() const { return _base[REG_SUM]; }
};


// CRC-CCITT
class CrcCCITT: public LpcCrc {
public:
    CrcCCITT(uint16_t initial = 0xffff)
        : LpcCrc(CRC_BASE)
    {
        LpcCrc::SetMode(LpcCrc::MODE_POLY_CCITT);
        LpcCrc::SetSeed(initial);
    }

    static uint16_t Checksum(const void* block, uint len, uint16_t initial = 0xffff) {
        CrcCCITT crc(initial);
        crc.Update((const uint8_t*)block, len);
        return crc.GetValue();
    }
};


// CRC16
class Crc16: public LpcCrc {
public:
    Crc16(uint16_t initial = 0)
        : LpcCrc(CRC_BASE)
    {
        LpcCrc::SetMode(LpcCrc::MODE_POLY_CRC16 | LpcCrc::MODE_BIT_RVS_WR
                        | LpcCrc::MODE_BIT_RVS_SUM);
        LpcCrc::SetSeed(initial);
    }

    static uint16_t Checksum(const void* block, uint len, uint16_t initial = 0) {
        Crc16 crc(initial);
        crc.Update((const uint8_t*)block, len);
        return crc.GetValue();
    }
};


// CRC32
class Crc32: public LpcCrc {
public:
    Crc32(uint32_t initial = 0xffffffff)
        : LpcCrc(CRC_BASE)
    {
        LpcCrc::SetMode(LpcCrc::MODE_POLY_CRC32 | LpcCrc::MODE_BIT_RVS_WR
                        | LpcCrc::MODE_BIT_RVS_SUM | LpcCrc::MODE_CMPL_SUM);
        LpcCrc::SetSeed(initial);
    }

    static uint32_t Checksum(const void* block, uint len, uint32_t initial = 0xffffffff) {
        Crc32 crc(initial);
        crc.Update((const uint8_t*)block, len);
        return crc.GetValue();
    }
};

#endif // HAVE_HW_CRC

#endif // _LPC_CRC_H_
