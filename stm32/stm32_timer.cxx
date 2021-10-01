// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "stm32_timer.h"
#include "thread.h"

template<typename Counter>
void Stm32Timer<Counter>::Interrupt(void* token) {
    ((Stm32Timer<Counter>*)token)->HandleInterrupt();
}

template<typename Counter>
void Stm32Timer<Counter>::RunTimer(Counter count) {
    Thread::IPL G(IPL_CLOCK);
    volatile uint32_t& dier = reg<volatile uint32_t>(Register::TIM_DIER);
    dier &= ~BIT(UIE);

    volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::TIM_CR1);
    cr1 = (cr1 & ~(BIT(CMS) | BIT(DIR) | BIT(CEN)))
        | BIT(OPM);

    reg<volatile uint32_t>(Register::TIM_PSC) = 0;
    reg<volatile uint32_t>(Register::TIM_ARR) = count;
    reg<volatile uint32_t>(Register::TIM_CNT) = 0;

    volatile uint32_t& egr = reg<volatile uint32_t>(Register::TIM_EGR);
    egr |= BIT(UG);  // Generates update event, so do this before enabling UIE

    volatile uint32_t& sr = reg<volatile uint32_t>(Register::TIM_SR);
    sr &= ~BIT(UIF);
    dier |= BIT(UIE);
    cr1 |= BIT(CEN);
}

template<typename Counter>
void Stm32Timer<Counter>::RunTimerFreq(uint32_t freq) {
    const uint32_t full_count = _timerclk / freq;
    Counter prescale = 0;
    uint32_t count = full_count;
    if (count > ~(Counter)0) {
        // Can only happen when Counter is uint16_t
        prescale = count >> 16; // PSC adds 1 to value
        count /= (prescale + 1);
    }

    Thread::IPL G(IPL_CLOCK);
    volatile uint32_t& dier = reg<volatile uint32_t>(Register::TIM_DIER);
    dier &= ~BIT(UIE);

    volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::TIM_CR1);
    cr1 &= ~(BIT(CMS) | BIT(DIR) | BIT(CEN) | BIT(OPM));

    reg<volatile uint32_t>(Register::TIM_PSC) = prescale;
    reg<volatile uint32_t>(Register::TIM_ARR) = count;
    reg<volatile uint32_t>(Register::TIM_CNT) = 0;

    volatile uint32_t& egr = reg<volatile uint32_t>(Register::TIM_EGR);
    egr |= BIT(UG);  // Generates update event, so do this before enabling UIE

    volatile uint32_t& sr = reg<volatile uint32_t>(Register::TIM_SR);
    sr &= ~BIT(UIF);
    dier |= BIT(UIE);
    cr1 |= BIT(CEN);
}


void Clock::Tick()
{
	_time += TIMEBASE / HZ;
}

template class Stm32Timer<uint32_t>;
template class Stm32Timer<uint16_t>;
