// Copyright (c) 2018-21 Jan Brittenson
// See LICENSE for details.

#include "params.h"
#include "core/enetkit.h"
#include "soc/stm32f4x/timer.h"
#include "core/thread.h"

template<typename Counter>
void Stm32Timer<Counter>::Interrupt(void* token) {
    ((Stm32Timer<Counter>*)token)->HandleInterrupt();
}

#define DIER reg(Register::TIM_DIER)
#define CR1  reg(Register::TIM_CR1)
#define CR2  reg(Register::TIM_CR2)
#define SR   reg(Register::TIM_SR)
#define PSC  reg(Register::TIM_PSC)
#define ARR  reg(Register::TIM_ARR)
#define CNT  reg(Register::TIM_CNT)
#define EGR  reg(Register::TIM_EGR)
#define CCER reg(Register::TIM_CCER)

template<typename Counter>
void Stm32Timer<Counter>::RunTimerFreq(uint32_t freq, uint32_t timebase, bool intr, bool trgo) {
    const uint32_t count = _timerclk / freq;
    const uint32_t prescale = count / timebase;
    const uint32_t full_count = count / prescale;

    assert(prescale <= 0xffff);
    assert(full_count <= (Counter)~0);

//    DMSG("freq: %u, timebase: %u, timerclk: %u", freq, timebase, _timerclk);
//    DMSG(" => count: %u, prescale: %u, full_count: %u", count, prescale, full_count);

    Thread::IPL G(IPL_CLOCK);
    DIER &= ~BIT(UIE);

    CR1 &= ~(BIT(CMS) | BIT(DIR) | BIT(CEN) | BIT(OPM));
    CR2 = trgo ? (0b010 << MMS) : 0;

    PSC = prescale;
    ARR = full_count;
    CNT = 0;

    EGR |= BIT(UG);  // Generates update event, so do this before enabling UIE

    SR &= ~BIT(UIF);
    if (intr)
        DIER |= BIT(UIE);
    CR1 |= BIT(CEN);
}

template<typename Counter>
void Stm32Timer<Counter>::RunPwm(uint32_t freq) {
    assert(PWM_PREC/freq <= (Counter)~0UL);

    const uint32_t count = _timerclk / freq;
    const uint32_t prescale = count / PWM_PREC;
    const uint32_t full_count = count / prescale;

    assert(prescale <= 0xffff);
    assert(full_count <= (Counter)~0);

    Thread::IPL G(IPL_CLOCK);
    DIER &= ~BIT(UIE);

    CR1 &= ~(BIT(CMS) | BIT(DIR) | BIT(CEN) | BIT(OPM));

    PSC = prescale;
    ARR = full_count;
    CNT = 0;

    CR1 |= BIT(ARPE);
    EGR |= BIT(UG);

    SR &= ~BIT(UIF);
    CR1 |= BIT(CEN);
}


template <typename Counter>
void Stm32Timer<Counter>::ConfigurePwm(Stm32Timer::CCR ccr, Stm32Timer::PwmPolarity pol) {
    const uint32_t ccer_shift = 4 * (uint32_t)ccr;

    CCER &= ~BIT(CC1E + ccer_shift);

    const uint32_t ccmbits = BIT(OC1PE) | ((uint32_t)PwmMode::MODE2 << OC1M);
    const uint32_t ccmr_shift = 8 * ((uint32_t)ccr & 1);
    volatile uint32_t& ccmr = reg(ccr >= CCR::CCR3 ? Register::TIM_CCMR2 : Register::TIM_CCMR1);
    ccmr = (ccmr & ~(0b11111111 << ccmr_shift))
        | (ccmbits << ccmr_shift);

    CCER = (CCER & ~(BIT(CC1E) | BIT(CC1P) | BIT(CC1NP)) << ccer_shift)
        | ((uint32_t)pol << ccer_shift);
    
    SetPwmDuty(ccr, PWM_OFF);

    CCER |= BIT(CC1E + ccer_shift);
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

// for -fwhole-file
#undef DIER
#undef CR1
#undef CR2
#undef SR
#undef PSC
#undef ARR
#undef CNT
#undef EGR
#undef CCER
