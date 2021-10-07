// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "lpc_pll.h"


void LpcPll::Init(uint32_t freq)
{
    const uint32_t cclk_mult = freq / FOSC;      // Desired PLL multiplier

	assert(cclk_mult >= 1);
	assert(cclk_mult <= 32);

	const int m = cclk_mult - 1;

	// Find a power-of 2 CCO divider in the range [1..8]
	// Begin at the smallest divider (i.e. max CCO)
	int p = CCO_MAX/(FOSC * cclk_mult * 2); // Nominal CCO divider
	assert(p <= 8);
	assert(p >= 0);

	// Bump divider upwards to next power of two (i.e. lower CCO)
	int tmp = p & (p - 1);
	if (tmp) {
		while (tmp)
            p = exch(tmp, p & (p - 1));

		p <<= 1;
	}

	const uint32_t cco = FOSC * cclk_mult * p * 2;
	assert(cco <= CCO_MAX);
	assert(cco >= CCO_MIN);

	const uint8_t pbit = Util::ffs(p);
	assert(pbit >= 0);
	assert(pbit <= 3);

	// Set up PLL and enable
	_base[REG_CFG] = (pbit << 5) | m;
	_base[REG_CON] = 1;
	Feed();

	// Wait for lock
	while (!(_base[REG_STAT] & BIT10)) 
        ;
}
