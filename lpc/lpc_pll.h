// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __LPC_PLL_H__
#define __LPC_PLL_H__

// FOSC should be set to oscillator, in Hz
// CCLK_MULT should be set to desired cpu clock multiplier (* FOSC)
// CCLK_MAX should be max CCLK
// CCLK will be defined as actual cpu clock
// PCLK will be defined as actual pclk

#define __MIN(A,B) ((A) < (B) ? (A) : (B))

class LpcPll {
	volatile uint32_t* _base;
public:
    enum {
        CCO_MIN = 156000000,        // Min CCO frequency
        CCO_MAX = 320000000,        // Max CCO frequency
    };

    enum {
        REG_CON = 0,
        REG_CFG = 1,
        REG_STAT = 2,
        REG_FEED = 3
    };

	LpcPll(uintptr_t base) : _base((volatile uint32_t*)base) { }

	void Init(uint32_t freq);
private:
	[[__finline]] void Feed() {
		_base[REG_FEED] = 0xaa;
		_base[REG_FEED] = 0x55;
	}
};

#undef __MIN

#endif // __LPC_PLL_H__
