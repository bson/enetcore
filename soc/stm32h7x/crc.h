// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef _CRC_H_
#define _CRC_H_

#ifndef HAVE_HW_CRC

// Software substitutions
#include "crc16.h"
#include "crc32.h"

#else

#include <stdint.h>
#include "compiler.h"
#include "core/bits.h"

class Crc {
    volatile uint32_t* _base;

    enum {
        DR    = 0x00/4,           // Data reg
        IDR   = 0x04/4,           // Independent DR (user scratch pad)
        CR    = 0x08/4,           // Control reg
        INIT  = 0x10/4,           // Initial value
        POL   = 0x14/4,           // Polynomial
    };

public:
    enum {
        // CR
        REV_OUT = 7,            // Reverse output
        REV_IN = 5,             // 2 bit, reverse input
        POLYSIZE = 3,           // 2 bit, polynomial size
        RESET = 0,              // Reset (restart, reinitialize)
    };

    enum {
        REV_IN_NO_REV = 0b00,   // No reversal
        REV_IN_BYTE_REV = 0b01, // Reverse bits by byte
        REV_IN_HW_REV = 0b10,   // Reverse bits by half word
    };
    
    // POLYSIZE
    enum {
        POLSZ_32 = 0b00,        // 32-bit polynomial
        POLSZ_16 = 0b01,        // 16-bit polynomial
        POLSZ_8 = 0b10,         // 8-bit polynomial
        POLSZ_7 = 0b11          // 7-bit polynomial
    };


    Crc(uintptr_t base)
        : _base((volatile uint32_t*)base) {
    }

    void SetMode(bool rev_out, uint8_t rev_in, uint8_t polysize) {
        _base[CR] = (((uint32_t)rev_out << REV_OUT) : 0)
            | (rev_in << REV_IN)
            | (polysize << POLYSIZE);
    }
    void SetInit(uint32_t init) { _base[INIT] = init; }
    void SetPoly(uint32_t poly) { _base[POLY] = poly; }
    void Reset() { _base[CR] |= BIT(RESET); }


    // Accepts anything 8, 16, or 32 bit.
    template <typename T>
    void Update(T data) { ((volatile T*)_base)[DR] = data; }

    template <typename T>
	void Update(const T* block, uint len) {
        while (len--)
            Update(*block++);
    }

    // Get sum
    uint32_t GetValue() const volatile { return _base[DR]; }
};


// CRC-CCITT/XMODEM
class CrcCCITT: public Crc {
public:
    CrcCCITT()
        : Crc(BASE_CRC)
    {
        Crc::SetMode(false, false, POLSZ_16);
        Crc::SetInit(0xffff);
        Crc::SetPoly(0x1021);
        Crc::Reset();
    }

    static uint16_t Checksum(const uint16_t* block, uint len) {
        CrcCCITT crc();
        crc.Update((const uint16_t*)block, len);
        return crc.GetValue();
    }
};


// CRC16
class Crc16: public Crc {
public:
    Crc16()
        : Crc(BASE_CRC)
    {
        Crc::SetMode(true, true, POLSZ_16);
        Crc::SetInit(0);
        Crc::SetPoly(0x1021);
        Crc::Reset();
    }

    static uint16_t Checksum(const uint16_t* block, uint len) {
        Crc16 crc();
        crc.Update(block, len);
        return crc.GetValue();
    }
};


// CRC32
class Crc32: public Crc {
public:
    Crc32(uint32_t initial, uint32_t poly)
        : Crc(BASE_CRC)
    {
        Crc::SetMode(false, false, POLSZ_32);
        Crc::SetPoly(poly);
        Crc::SetInit(initial);
        Crc::Reset();
    }

    static uint32_t Checksum(const uint32_t* block,
                             uint len,
                             uint32_t initial = 0xffffffff,
                             uint32_t poly = 0x04c11db7) {
        Crc32 crc(initial, poly);
        crc.Update(block, len);
        return crc.GetValue();
    }
};

#endif // HAVE_HW_CRC

#endif // _CRC_H_
