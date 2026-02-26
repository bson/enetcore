#ifndef __FSM_H__
#define __FSM_H__

#include "core/bits.h"

class Stm32Fmc {
public:
    enum class Bank {
        BANK1 = 0x00,
        BANK2 = 0x08,
        BANK3 = 0x10,
        BANK4 = 0x18
    };

private:
    enum class Register {
        BCR1 = 0x0000,
        BCR2 = 0x0008,
        BCR3 = 0x0010,
        BCR4 = 0x0018,
        BTR1 = 0x0004,
        BTR2 = 0x000c,
        BTR3 = 0x0014,
        BTR4 = 0x001c,
        BWTR1 = 0x0104,
        BWTR2 = 0x010c,
        BWTR3 = 0x0114,
        BWTR4 = 0x011c,

        // NAND flash (write FIFO)
        PCR = 0x0080,
        SR  = 0x0084,
        PMEM = 0x0088,
        PATT = 0x008c,
        ECCR = 0x0094,

        // SDRAM controller
        SDCR1 = 0x0140,
        SDCR2 = 0x0144,
        SDTR1 = 0x0148,
        SDTR2 = 0x014c,
        SDCMR = 0x0150,
        SDRTR = 0x0154,
        SDSR = 0x0158,

    };

    enum {
        // BCRx
        FMCEN = 31,
        BMAP = 24,
        WFDIS = 21,
        CCLKEN = 20,
        CBURSTRW = 19,
        CPSIZE = 16,
        ASYNCWAIT = 15,
        EXTMOD = 14,
        WAITEN = 13,
        WREN = 12,
        WAITCFG = 11,
        WRAPMOD = 10,
        WAITPOL = 9,
        BURSTEN = 8,
        FACCEN = 6,
        MWID = 4,
        MTYP = 2,
        MUXEN = 1,
        MBKEN = 0,

        // BTRx, BWTRx
        ACCMOD = 28,
        DATLAT = 24,            // SPRAM only, not SRAM/NOR
        CLKDIV = 20,            // SPRAM only, not SRAM/NOR
        BUSTURN = 16,
        DATAST = 8,
        ADDHLD = 4,
        ADDSET = 0,

        // PCRx
        ECCPS = 17,
        TAR = 13,
        TCLR = 9,
        ECCEN = 6,
        PWID = 4,
        PBKEN = 2,
        PWAITEN = 1,

        // SRx
        FEMPT = 6,
        IFEN = 5,
        ILEN = 4,
        IREN = 3,
        IFS = 2,
        ILS = 1,
        IRS = 0,

        // MEMx
        MEMHIZ = 24,
        MEMHOLD = 16,
        MEMWAIT = 8,
        MEMSET = 0
    };
    
    enum Width {
        BYTE = 0b00,
        WORD16 = 0b01
    };

    enum Type {
        SRAM = 0b00,
        PSRAM = 0b01,
        NOR_FLASH = 0b10
    };

    static volatile uint32_t& reg(Register r) {
        return *(volatile uint32_t*)(BASE_FMC + (uint32_t)r);
    }

    static volatile uint32_t& reg(Bank b, Register r) {
        return *(volatile uint32_t*)(BASE_FMC + (uint32_t)r + (uint32_t)b);
    }

public:
    static void ConfigureSRAM(Bank b, uint16_t data_setup, uint16_t data_hold, uint16_t bus_turn) {
        if (bus_turn > 15)
            bus_turn = 15;
        assert(data_setup <= 15);
        assert(data_hold >= 1 && data_hold <= 255);

        volatile uint32_t& b_bcr = reg(b, Register::BCR1); // Bank BCR
        volatile uint32_t& b_btr = reg(b, Register::BTR1); // Bank BTR
        volatile uint32_t& bcr1 = reg(Register::BCR1);     // BCR1

        bcr1 = Bitfield(bcr1)
            .bit(FMCEN);

        b_bcr = Bitfield(b_bcr)
            .cbit(MBKEN);

        b_bcr = Bitfield(b_bcr)
            .bit(WREN)
            .f(2, MWID, Width::WORD16)
            .f(2, MTYP, Type::SRAM); // F405 used Type::PSRAM?!

        b_btr = Bitfield(b_btr)
            .f(4, BUSTURN, bus_turn)
            .f(4, ADDSET, data_setup)
            .f(8, DATAST, data_hold);

        b_bcr = Bitfield(b_bcr)
            .bit(MBKEN);
    }
};


#endif // __FSM_H__
