// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "params.h"
#include "enetkit.h"
#include "stm32f4x/timer.h"
#include "thread.h"

template<typename Counter>
void Stm32Timer<Counter>::Interrupt(void* token) {
    ((Stm32Timer<Counter>*)token)->HandleInterrupt();
}

#define DIER reg<volatile uint32_t>(Register::TIM_DIER)
#define CR1  reg<volatile uint32_t>(Register::TIM_CR1)
#define SR   reg<volatile uint32_t>(Register::TIM_SR)
#define PSC  reg<volatile uint32_t>(Register::TIM_PSC)
#define ARR  reg<volatile uint32_t>(Register::TIM_ARR)
#define CNT  reg<volatile uint32_t>(Register::TIM_CNT)
#define EGR  reg<volatile uint32_t>(Register::TIM_EGR)

template<typename Counter>
void Stm32Timer<Counter>::RunTimerFreq(uint32_t freq, uint32_t prec) {
    assert(prec/freq <= ~(Counter)0);

    const uint32_t count = _timerclk / freq;
    const Counter prescale = count / prec;
    const uint32_t full_count = count / prescale;

    assert(prescale <= ~(Counter)0);
    assert(full_count <= ~(Counter)0);

    Thread::IPL G(IPL_CLOCK);
    DIER &= ~BIT(UIE);

    CR1 &= ~(BIT(CMS) | BIT(DIR) | BIT(CEN) | BIT(OPM));

    PSC = prescale;
    ARR = count;
    CNT = 0;

    EGR |= BIT(UG);  // Generates update event, so do this before enabling UIE

    SR &= ~BIT(UIF);
    DIER |= BIT(UIE);
    CR1 |= BIT(CEN);
}

template <typename Counter>
inline void Stm32Timer<Counter>::HandleInterrupt() {
    if (SR & BIT(UIF)) {
        Tick();
        SR &= ~BIT(UIF);
    }
}

void Clock::Start() {
    ScopedNoInt G();

    Stm32Timer::RunTimerFreq(CLOCK_HZ, TIMEBASE);
}

void Clock::Tick()
{
	_time += TIMEBASE / CLOCK_HZ;
}

uint64_t Clock::GetTime() {
    ScopedNoInt G();

    // Because clock interrupts are low priority we may have had an ARR event
    // that has not yet been processed.  So handle it here now.
    if (SR & BIT(UIF))
        Stm32Timer::HandleInterrupt();

    return _time + GetCount();
}


template class Stm32Timer<uint32_t>;
template class Stm32Timer<uint16_t>;
