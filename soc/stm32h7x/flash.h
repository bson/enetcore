// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_FLASH_H__
#define __STM32_FLASH_H__

#include "core/bits.h"
#include "core/bitfield.h"


// Basic flash support.  No erase or write support.
class Stm32Flash {
    // All registers that end with '1' replicate for bank 2 at offset 0x100
    enum Register {
        ACR = 0x00,
        KEYR1 = 0x04,
        OPTKEYR = 0x08,
        CR1 = 0x0c,
        SR1 = 0x10,
        CCR1 = 0x14,
        OPTCR = 0x18,
        OPTSR_CUR = 0x1c,
        OPTSR_PRG = 0x20,
        OPTCCR = 0x24,
        PRAR_CUR1 = 0x28,
        PRAR_PRG1 = 0x2c,
        SCAR_CUR1 = 0x30,
        SCAR_PRG1 = 0x34,
        WPSN_CUR1R = 0x38,
        WPSN_PRG1R = 0x3C,
        BOOT_CURR = 0x40,
        BOOT_PRGR = 0x44,
        CRCCR1 = 0x50,
        CRCADD1R = 0x54,
        CRCEADD1R = 0x58,
        CRCDATAR = 0x5C,
        RCC_FA1R = 0x60,
    };

public:
    enum {
        SECTOR_SIZE = 128*1024,
        BANK_SIZE = 1024*1024 - SECTOR_SIZE,
    };

    enum {
        // FLASH_ACR, reset 0x0000 0000
        WRHIGHFREQ = 4,
        LATENCY = 0,

        // FLASH_SR1/2
        QW1 = 2,
    };

    enum {
        BANK1 = 0x000,
        BANK2 = 0x100
   };

private:
    // Bank-specific register
    static volatile uint32_t& reg(const Bank bank, const Register r) {
        return *((volatile uint32_t*)(BASE_FLASH + uint32_t(bank) + uint32_t(r)));
    }

    // Common register
    static volatile uint32_t& creg(const Register r) {
        return *((volatile uint32_t*)(BASE_FLASH + uint32_t(r)));
    }


public:
    class Access {
        Bank _bank;

    public:
        Access(Bank bank)
            : _bank(bank)
        {
            // CR1
            reg(bank, Register::KEYR1) = 0x45670123;
            reg(bank, Register::KEYR1) = 0xcdef89ab;

            // OPTCR
            reg(bank, Register::OPTKEYR) = 0x08192a3b;
            reg(bank, Register::OPTKEYR) = 0x4c5d6e7f;;

        }

        ~Access() {
            // CR1
            reg(_bank, Register::KEYR1) = 0;

            // OPTCR
            reg(_bank, Register::OPTKEYR) = 0;;
        }

    };

    // Initialize
    static void Init() {
        reg(Register::ACR) = Bitfield(reg(Register::ACR))
            .f(2, WRHIGHFREQ, 2); // Max WS... XXX fixme
    }

    // Current bank
    static Bank GetBank() const {
        return creg(Register::OPTSR_CUR) & BIT(SWAP_BANK_OPT) ? Bank::BANK2 : Bank::BANK1;
    }

    static void Latency(Bank bank, uint32_t latency) {
        reg(Register::ACR) = Bitfield(reg(Bank::BANK0, Register::ACR))
            .f(0, LATENCY, latency);
        reg(Register::ACR) = Bitfield(reg(Bank::BANK1, Register::ACR))
            .f(0, LATENCY, latency);
    }

    // Cortex-M7 has unified cache management
    static void EnableIDCaching() { }

    // XXX add programming
};

#endif // __STM32_FLASH_H__
