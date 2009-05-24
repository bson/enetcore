#ifndef __LPC_TIMER_H__
#define __LPC_TIMER_H__


class LpcTimer {
	volatile uint32_t* const _base;
	mutable Spinlock _lock;
	uint8_t _resolution;

public:
	LpcTimer(uintptr_t base) :
		_base((volatile uint32_t*)base)
	{ }

	// Set time resolution
	void SetResolution(uint8_t r);

	// Start timer - will interrupt
	void RunTimer(uint count, uint mr);	// Run once up to count
	void RunTimerFreq(uint freq, uint mr); // Run repeatedly, trigger at freq Hz

	// Read timer
	uint GetCount() const { Spinlock::Scoped L(_lock); return _base[TIMER_TC]; }

	// Interrupt handler
	static void Interrupt() __irq __naked;

	void HandleInterrupt(uint mr);

	// Called from interrupt handler to indicate clock tick
	virtual void Tick() = 0;

private:
	LpcTimer();
};


// System clock
class Clock: public LpcTimer {
	uint64_t _time;
public:
	Clock() : LpcTimer(TIMER0_BASE), _time(0) { }

	uint64_t GetTime() const { return _time + GetCount(); }

	void Tick();
};


// System timer
class SysTimer: public LpcTimer {
public:
	SysTimer() : LpcTimer(TIMER1_BASE) { }
	void Tick();
	void SetTimer(uint usec) { RunTimer(usec, 1); }
};

extern Clock _clock;
extern SysTimer _systimer;

#endif // __LPC_TIMER_H__
