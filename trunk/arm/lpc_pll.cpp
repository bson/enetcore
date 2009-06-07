#include "enetkit.h"
#include "lpc_pll.h"


void LpcPll::Init()
{
	assert(CCLK_MULT >= 1);
	assert(CCLK_MULT <= 32);
	assert(FOSC * CCLK_MULT <= CCLK_MAX);

	const int m = CCLK_MULT - 1;

	// Find a power-of 2 CCO divider in the range [1..8]
	// Begin at the smallest divider (i.e. max CCO)
	int p = CCO_MAX/(FOSC * CCLK_MULT * 2); // Nominal CCO divider
	assert(p <= 8);
	assert(p >= 0);

	// Bump divider upwards to next power of two (i.e. lower CCO)
	int tmp = p & (p - 1);
	if (tmp) {
		while (tmp)  p = exch(tmp, p & (p - 1));
		p <<= 1;
	}
	assert(p <= 8);
	assert(p > 0);

	const uint cco = FOSC * CCLK_MULT * p * m * 2;
	assert(cco <= CCO_MAX);
	assert(cco >= CCO_MIN);

	const uint8_t pbit = Util::ffs(p);
	assert(pbit >= 0);
	assert(pbit <= 3);

	// Set up PLL
	_base[PLL_CFG] = (pbit << 5) | m;
	Feed();

	// Start
	_base[PLL_CON] = 1;
	Feed();

	// Wait for lock
	while (!(_base[PLL_STAT] & 0x400)) continue;

	// Connect to CCLK
	_base[PLL_CON] = 3;
	Feed();

	// PCLK = CCLK
	VPBDIV = 1;
}
