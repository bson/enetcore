// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "lpc_timer.h"
#include "thread.h"


void LpcTimer::SetResolution(uint8_t r)
{
	_resolution = r;
}


void LpcTimer::RunTimerFreq(uint freq)
{
	const uint prescale = PCLK / (1 << _resolution);
	const uint match = (1 << _resolution) / freq;

    ScopedNoInt G;

    _base[REG_CCR] = 0;
    _base[REG_EMR] = 0;
    _base[REG_CTRC]= 0;

	_base[REG_TCR] = 0b10;

	_base[REG_PR] = prescale - 1;
	_base[REG_MR0 + _mr] = match;

	// Interrupt and reset TC on match
	_base[REG_MCR] = 0b011 << (_mr * 3);
	
	_base[REG_TCR] = 0b01;
}


void LpcTimer::RunTimer(uint count)
{
	const uint prescale = PCLK / (1 << _resolution);

    ScopedNoInt G;

    _base[REG_CCR] = 0;
    _base[REG_EMR] = 0;
    _base[REG_CTRC]= 0;

	_base[REG_TCR] = 0b10;

	_base[REG_PR] = prescale - 1;
	_base[REG_MR0 + _mr] = count;

	// Stop and interrupt timer on match
	_base[REG_MCR] = 0b101 << (_mr * 3);
	
	_base[REG_TCR] = 0b01;
}


void LpcTimer::Interrupt(void* token) {
    LpcTimer* timer = (LpcTimer*)token;

    timer->HandleInterrupt();
}


void LpcTimer::HandleInterrupt()
{
	if (_base[REG_IR] & (1 << _mr)) {
		Tick();
		_base[REG_IR] = (1 << _mr);
	}
}


void Clock::Tick()
{
	_time += TIMEBASE / HZ;
}
