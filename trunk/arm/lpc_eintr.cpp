#include "enetkit.h"


LpcEintr::LpcEintr(uint num, uint flags) :
	_intch(INTCH_EINT0 + num),
	_extint(1 << num)
{
	assert(num <= 63);
	assert(_extint <= 8);
	assert(flags <= 7);

	_wake = flags & EINTR_WAKE;
	_low = flags & EINTR_LOW;
	_level = flags & EINTR_LEVEL;
}


void LpcEintr::Init()
{
	EXTWAKE = (EXTWAKE & ~_extint) | (_wake ? _extint : 0);
	VPBDIV=0;				   // Workaround for EXTINT.1, EXTINT.2 errata
	EXTPOLAR = (EXTPOLAR & ~_extint) | (_low ? 0 : _extint);
	VPBDIV=1;				   // CPU bug workaround
	VPBDIV=0;				   // CPU bug workaround
	EXTMODE = (EXTMODE & ~_extint) | (_level ? 0 : _extint);
	VPBDIV=1;				   // CPU bug workaround
	VPBDIV=1;				   // CPU bug workaround
	EXTINT = _extint;		   // Clear any stray EINTx flag
}


void LpcEintr::Enable(bool enable)
{
	if (enable)
		_vic.EnableChannel(_intch);
	else
		_vic.DisableChannel(_intch);
}


void LpcEintr::InstallHandler(IRQHandler handler)
{
	_vic.InstallHandler(_intch, handler);
	Init();
}


// * static __irq __naked
void LpcEintr::Interrupt()
{
	SaveStateExc(4);

	if (_eintr0.Pending()) {
		_eintr0.HandleInterrupt();
		_eintr0.ClearPending();
	}

	if (_eintr1.Pending()) {
		_eintr1.HandleInterrupt();
		_eintr1.ClearPending();
	}

	if (_eintr2.Pending()) {
		_eintr2.HandleInterrupt();
		_eintr2.ClearPending();
	}

	if (_eintr3.Pending()) {
		_eintr3.HandleInterrupt();
		_eintr3.ClearPending();
	}

	_vic.ClearPending();
	LoadStateReturnExc();
}


bool LpcEintr::Pending()
{
	return _vic.ChannelPending(_intch);
}


void LpcEintr::ClearPending()
{
	_vic.DisableChannel(_intch);
	EXTINT = _extint;			// Clear EINT flag
	_vic.EnableChannel(_intch);
}
