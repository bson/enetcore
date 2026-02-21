// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#include "eeprom.h"
#include "bits.h"


void Stm32Eeprom::Write(uint16_t addr, const uint32_t* block, uint16_t len) {
    assert_bounds(addr + len < EEPROM_SIZE);
    assert((addr & (sizeof *block - 1)) == 0);    // Integral addr

    Access a1_(BANK1), a2_(BANK2);

    Stm32Flash::reg(BANK1, CR1) |= BIT(PG1);
    Stm32Flash::reg(BANK2, CR1) |= BIT(PG1);

    uint32_t remain = len / sizeof *block;
    uint32_t *pos1 = (uint32_t*)(BASE_FLASHB1 + Stm32Flash::BANK_SIZE - EEPROM_SIZE + addr);
    uint32_t *pos2 = (uint32_t*)(BASE_FLASHB2 + Stm32Flash::BANK_SIZE - EEPROM_SIZE + addr);

    if (remain--) {
        *pos1++ = *block;
        *pos2++ = *block++;

        while ((Stm32Flash::reg(BANK1, SR1) & BIT(QW1)) == 0
               || (Stm32Flash::reg(BANK2, SR1) & BIT(QW2)) == 0)
            ;
    }

    Stm32Flash::reg(BANK1, CR1) &= ~BIT(PG1);
    Stm32Flash::reg(BANK2, CR1) &= ~BIT(PG1);
}

