#ifndef __FSMC_H__
#define __FSMC_H__

#include "core/bits.h"

class Stm32Fsmc {
public:
    enum class Bank {
        BANK1 = 0,
        BANK2 = 8,
        BANK3 = 16,
        BANK4 = 14
    };

private:
    enum class Register {
        BCR  = 0x0000,
        BCR1 = 0x0000,
        BCR2 = 0x0008,
        BCR3 = 0x0010,
        BCR4 = 0x0018,
        BTR  = 0x0004,
        BTR1 = 0x0004,
        BTR2 = 0x000c,
        BTR3 = 0x0014,
        BTR4 = 0x001c,
        BWTR  = 0x0104,
        BWTR1 = 0x0104,
        BWTR2 = 0x010c,
        BWTR3 = 0x0114,
        BWTR4 = 0x011c,
        PCR2 = 0x0060,
        PCR3 = 0x0080,
        PCR4 = 0x00a0,
        SR2 = 0x0064,
        SR3 = 0x0084,
        SR4 = 0x00a4,
        PMEM2 = 0x0068,
        PMEM3 = 0x0088
    };

    enum {
        // BCRx
        CBURSTRW = 19,
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
        DATLAT = 24,
        CLKDIV = 20,
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
        PTYP = 3,
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
        return *(volatile uint32_t*)(BASE_FSMC + (uint32_t)r);
    }

    static volatile uint32_t& reg(Bank b, Register r) {
        return *(volatile uint32_t*)(BASE_FSMC + (uint32_t)r + (uint32_t)b);
    }

public:
    static void ConfigureSRAM(Bank b, uint16_t data_setup, uint16_t data_hold, uint16_t bus_turn) {
        if (bus_turn > 15)
            bus_turn = 15;
        assert(data_setup <= 15);
        assert(data_hold >= 1 && data_hold <= 255);

        reg(b, Register::BCR) &= ~BIT(MBKEN);
        reg(b, Register::BCR) = BIT(WREN) | (Width::WORD16 << MWID) | (Type::SRAM << MTYP)
            | BIT(7);
        reg(b, Register::BTR) = (bus_turn << BUSTURN) | (data_setup << ADDSET)
            | (data_hold << DATAST);
        reg(b, Register::BCR) |= BIT(MBKEN);
    }
};


#endif // __FSMC_H__
