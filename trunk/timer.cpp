#include "enetkit.h"
#include "timer.h"
#include "thread.h"


void Timer::SetResolution(uint8_t r)
{
	_resolution = r;
}


void Timer::RunTimerFreq(uint freq, uint mr)
{
	const uint prescale = PCLK / (1 << _resolution);
	const uint match = (1 << _resolution) / freq;

	Spinlock::Scoped L(_lock);
	_base[TIMER_TCR] = 0b10;

	_base[TIMER_PR] = prescale - 1;
	_base[TIMER_MR0+mr] = match;

	// Clear and reset TC on match
	_base[TIMER_MCR] = 0b011 << (mr * 3);
	
	_base[TIMER_TCR] = 0b01;
}



void Timer::RunTimer(uint count, uint mr)
{
	const uint prescale = PCLK / (1 << _resolution);

	Spinlock::Scoped L(_lock);
	_base[TIMER_TCR] = 0b10;

	_base[TIMER_PR] = prescale - 1;
	_base[TIMER_MR0+mr] = count;

	// Stop timer on match
	_base[TIMER_MCR] = 0b101 << (mr * 3);
	
	_base[TIMER_TCR] = 0b01;
}



// * static __irq NAKED
void Timer::Interrupt()
{
	SaveStateExc(4);

	if (_vic.ChannelPending(INTCH_TIMER0))
		_clock.HandleInterrupt(0); // Timer0 uses MR0

	if (_vic.ChannelPending(INTCH_TIMER1))
		_systimer.HandleInterrupt(1); // Timer1 uses MR1

	_vic.ClearPending();

	LoadStateReturnExc();		// Load _curthread and return
}


void Timer::HandleInterrupt(uint mr)
{
	Spinlock::Scoped L(_lock);
	if (_base[TIMER_IR] & (1 << mr)) {
		Tick();
		_base[TIMER_IR] = (1 << mr);
	}
}


void Clock::Tick()
{
	_time += TIMEBASE / HZ;
}
