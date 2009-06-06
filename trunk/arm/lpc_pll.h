#ifndef __LPC_PLL_H__
#define __LPC_PLL_H__

// FOSC should be set to oscillator, in Hz
// CCLK_MULT should be set to desired cpu clock multiplier (* FOSC)
// CCLK_MAX should be max CCLK
// CCLK will be defined as actual cpu clock
// PCLK will be defined as actual pclk

#define __MIN(A,B) ((A) < (B) ? (A) : (B))

enum { CCO_MIN = 156000000 };	// Min CCO frequency
enum { CCO_MAX = 320000000 };	// Max CCO frequency

enum { CCLK = __MIN(FOSC * CCLK_MULT, CCLK_MAX) };
enum { PCLK = CCLK };

enum { PLL_CON = 0,
	   PLL_CFG = 1,
	   PLL_STAT = 2,
	   PLL_FEED = 3
};


class LpcPll {
	volatile uint32_t* _base;
public:
	LpcPll(uintptr_t base) : _base((volatile uint32_t*)base) { }

	void Init();
private:
	inline void Feed() __finline {
		_base[PLL_FEED] = 0xaa;
		_base[PLL_FEED] = 0x55;
	}

};

#undef __MIN

#endif // __LPC_PLL_H__
