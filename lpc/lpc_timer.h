// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __LPC_TIMER_H__
#define __LPC_TIMER_H__

#include <stdint.h>
#include "lpc.h"


class LpcTimer {
	volatile uint32_t* _base;
	uint8_t _resolution;
    uint8_t _irq;
    uint8_t _mr;                // Match register to use

    enum { 
        REG_IR   = 0,
        REG_TCR  = 0x04/4,
        REG_TC   = 0x08/4,
        REG_PR   = 0x0c/4,
        REG_PC   = 0x10/4,
        REG_MCR  = 0x14/4,
        REG_MR0  = 0x18/4,
        REG_MR1  = 0x1c/4,
        REG_MR2  = 0x20/4,
        REG_MR3  = 0x24/4,
        REG_CCR  = 0x28/4,
        REG_CR0  = 0x2c/4,
        REG_CR1  = 0x30/4,
        REG_EMR  = 0x3c/4,
        REG_CTRC = 0x70/4
    };

public:
	LpcTimer(uintptr_t base, uint8_t irq, uint8_t mr)
        : _base((volatile uint32_t*)base),
          _resolution(20),
          _irq(irq),
          _mr(mr)
	{ }

	// Set timer resolution, in bits
	void SetResolution(uint8_t r);

	// Start timer - will interrupt
	void RunTimer(uint count);	// Run once up to count
	void RunTimerFreq(uint freq); // Run repeatedly, trigger Tick() at Hz

	// Read timer
	uint GetCount() const { return _base[REG_TC]; }

	// Interrupt handler
	static void Interrupt(void* token);

	// Called from interrupt handler to indicate clock tick
	virtual void Tick() = 0;

private:
	[[__finline]] inline void HandleInterrupt();

private:
	LpcTimer();
};


// System clock
// XXX use SYSTICK for this.  It's just a simple repeating count!
class Clock: public LpcTimer {
	uint64_t _time;
public:
	Clock() : LpcTimer(CLOCK_BASE, CLOCK_IRQ, 0), _time(0) { }

	uint64_t GetTime() const { return _time + GetCount(); }

	void Tick();
};


#endif // __LPC_TIMER_H__
