// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "board.h"

LpcEintr::LpcEintr(uint num, uint flags) :
	_irq(EINT0_IRQ + num),
	_extint(1 << num)
{
	assert(_irq <= INT_NUM);
	assert(_extint <= 8);
	assert(flags <= 7);

	_low = flags & EINTR_LOW;
	_level = flags & EINTR_LEVEL;
}


void LpcEintr::Init()
{
	EXTPOLAR = (EXTPOLAR & ~_extint) | (_low ? 0 : _extint);
	EXTMODE = (EXTMODE & ~_extint) | (_level ? 0 : _extint);
	EXTINT = _extint;		   // Clear any stray EINTx flag
}


void LpcEintr::Interrupt(void* token)
{
    LpcEintr* eintr = (LpcEintr*)token;

    eintr->EintrInterrupt();
    EXTINT = eintr->_extint;			// Clear EINT flag
}
