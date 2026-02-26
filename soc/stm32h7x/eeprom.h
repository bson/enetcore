// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _STM32_EEPROM_H_
#define _STm32_EEPROM_H_

#include <stdint.h>
#include "soc/stm32h7x.h"
#include "core/bits.h"
#include "soc/stm32h7x/flash.h"


// Writing of EEPROM for settings storage.
//
// In the STM32H7 we emulate this using the top 2k of the two flash
// banks.  The block is replicated in the two banks, with the current
// bank active and fallback to the other bank for backup.  The two
// replicas swap on a bank swap.
//
// Note that there is no read functionality since the flash is memory
// mapped.

class Stm32Eeprom: public Stm32Flash {
    enum {
        // 2k set aside at top of bank, in last sector
        EEPROM_SIZE = 2048
    };

public:
    Stm32Eeprom() { }

    // Initialize
    void Init() { }

    // Write block of data.  Returns true on success.  'len' is a byte
    // count.  Addr is the start address in the faux EEPROM.
    void Write(uint16_t addr, const uint32_t* block, uint16_t len);

    // Return base address
    void* Base() {
        return (Stm32Flash::GetBank() == Stm32Flash::Bank::BANK1
                ? (void*)BASE_FLASHB1 : (void*)BASE_FLASHB2);
    }

private:
    // Yeah, no
    Stm32Eeprom(const Stm32Eeprom&) = delete;
    Stm32Eeprom& operator=(const Stm32Eeprom&) = delete;
};


#endif // _STM32_EEPROM_H_
