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
void Stm32Timer<Counter>::RunTimer(Counter count) {
    Thread::IPL G(IPL_CLOCK);
    DIER &= ~BIT(UIE);

    CR1 = (CR1 & ~(BIT(CMS) | BIT(DIR) | BIT(CEN)))
        | BIT(OPM);

    PSC = 0;
    ARR = count;
    CNT = 0;

    EGR |= BIT(UG);  // Generates update event, so do this before enabling UIE

    SR &= ~BIT(UIF);
    DIER |= BIT(UIE);
    CR1 |= BIT(CEN);
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

void Clock::Tick()
{
	_time += TIMEBASE / CLOCK_HZ;
}

template class Stm32Timer<uint32_t>;
template class Stm32Timer<uint16_t>;
