#include "enetkit.h"
#include "timer.h"
#include "thread.h"


Clock _clock;


void Timer::SetResolution(uint8_t r)
{
	_resolution = r;
}


void Timer::RunTimer(uint count, bool recur)
{
	const uint prescale = PCLK / (1 << _resolution);
	const uint match = (1 << _resolution) / count;

	Spinlock::Scoped L(_lock);
	_base[TIMER_TCR] = 0b10;

	_base[TIMER_PR] = prescale - 1;
	_base[TIMER_MR0] = match;

	// If recurring, clear and reload TC on match
	// If one-shot, stop timer on match
	_base[TIMER_MCR] = recur ? 0b000000000011 : 0b000000000101;
	
	_base[TIMER_TCR] = 0b01;
}



// * static __irq
void Timer::Interrupt()
{
//	SaveStateExc(4);

	if (_vic.ChannelPending(4))
		_clock.HandleInterrupt(0); // Timer0 uses MR0
#if 0
	if (_vic.ChannelPending(5))
		_timer1.HandleInterrupt(1); // Timer1 used MR1
#endif

	_vic.ClearPending();

//	Thread::LoadStateExc();
}


void Timer::HandleInterrupt(uint mr)
{
	Spinlock::Scoped L(_lock);
	if (_base[TIMER_IR] & (1 << mr)) {
		Tick();
		_base[TIMER_IR] = (1 << mr);
	}
}


// * implements Clock::GetTime
void Clock::Tick()
{
	_time += TIMEBASE / HZ;
}
