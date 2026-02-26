// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#include "params.h"
#include "core/enetkit.h"

#include "eeprom.h"
#include "core/bits.h"


void Stm32Eeprom::Write(uint16_t addr, const uint32_t* block, uint16_t len) {
    assert_bounds(addr + len < EEPROM_SIZE);
    assert((addr & (sizeof *block - 1)) == 0);    // Integral addr

    Access a1_(Stm32Flash::Bank::BANK1), a2_(Stm32Flash::Bank::BANK2);

    Stm32Flash::reg(Stm32Flash::Bank::BANK1, Stm32Flash::Register::CR1) |= BIT(Stm32Flash::PG1);
    Stm32Flash::reg(Stm32Flash::Bank::BANK2, Stm32Flash::Register::CR1) |= BIT(Stm32Flash::PG1);

    uint32_t remain = len / sizeof *block;
    uint32_t *pos1 = (uint32_t*)(BASE_FLASHB1 + Stm32Flash::BANK_SIZE - EEPROM_SIZE + addr);
    uint32_t *pos2 = (uint32_t*)(BASE_FLASHB2 + Stm32Flash::BANK_SIZE - EEPROM_SIZE + addr);

    if (remain--) {
        *pos1++ = *block;
        *pos2++ = *block++;

        while ((Stm32Flash::reg(Stm32Flash::Bank::BANK1, Stm32Flash::Register::SR1)
                & BIT(Stm32Flash::QW1)) == 0
               || (Stm32Flash::reg(Stm32Flash::Bank::BANK2, Stm32Flash::Register::SR1)
                   & BIT(Stm32Flash::QW1)) == 0)
            ;
    }

    Stm32Flash::reg(Stm32Flash::Bank::BANK1, Stm32Flash::Register::CR1) &= ~BIT(Stm32Flash::PG1);
    Stm32Flash::reg(Stm32Flash::Bank::BANK2, Stm32Flash::Register::CR1) &= ~BIT(Stm32Flash::PG1);
}

