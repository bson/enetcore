// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "lpc_pwm.h"


void LpcPwm::SetPeriodFreq(uint freq) {
    const uint prescale = PCLK / RESOLUTION;

	_match = RESOLUTION / uint32_t(freq);

    ScopedNoInt G;

    _base[REG_CCR] = 0;
	_base[REG_TCR] = BIT1;
    _base[REG_PC]  = 0;
	_base[REG_PR]  = prescale - 1;
	_base[REG_MR0] = _match;
    _base[REG_MCR] = BIT1;      // Reset TC on MR0 match (period)
    _base[REG_CCR] = 0;
    _base[REG_LER] = BIT0;      // Enable latching for MR0
    _base[REG_CTCR]= 0;         // Counter mode
    _base[REG_PCR] = 0;
};


void LpcPwm::SetDuty(uint8_t frac, uint n) {
    static const uint8_t regmap[6] = {
        REG_MR1, REG_MR2, REG_MR3, REG_MR4, REG_MR5, REG_MR6
    };

    assert_bounds(n > 0 && n <= 6);

    const uint8_t mr_reg = regmap[n-1];

    _base[mr_reg]   = _match * uint32_t(frac) / 256;
	_base[REG_TCR]  = BIT3 | BIT0;
    _base[REG_PCR] |= 1 << (n + 8); // Enable output
    _base[REG_LER]  = 1 << n;       // Latch MRn on next period
}
