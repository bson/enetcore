#ifndef __TIMER_H__
#define __TIMER_H__


class Timer {
	volatile uint32_t* const _base;
	mutable Spinlock _lock;
	uint8_t _resolution;

public:
	Timer(uint32_t base) :
		_base((volatile uint32_t*)base)
	{ }

	// Set time resolution
	void SetResolution(uint8_t r);

	// Start timer - will interrupt
	void RunTimer(uint count, bool recur);

	// Read timer
	uint GetCount() const { return _base[TIMER_TC]; }

	// Interrupt handler
	static void Interrupt() __irq;

	void HandleInterrupt();

	// Called from interrupt handler to indicate clock tick
	virtual void Tick() = 0;

private:
	Timer();
};


// System clock
class Clock: public Timer {
	uint64_t _time;
public:
	Clock() : Timer(TIMER0_BASE), _time(0) { }

	uint64_t GetTime() const {
		return _time + GetCount();
	}

	void Tick();
};


extern Clock _clock;

#endif // __TIMER_H__
