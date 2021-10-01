#ifndef __STM32_TIMER_H__
#define __STM32_TIMER_H__

// There are four kinds of timers:
//  General purpose 1
//     TIM2, 3, 4, 5
//     TIM2 and 5 are 32-bit
//     4 CCRs
//     These are all on APB1
//     Count up, down, or up-down
//     Supports quadrature encoding
//
//  General purpose 2
//     TIM9-14
//     All 16-bit
//     2 CCRs
//     TIM9-11 on APB2
//     TIM12-14 on APB1
//
//  Basic timer
//     TIM6,7
//     16-bit
//     Up count only, reload, ARR
//     No CCRs
//     On APB1
//     Mainly intended to drive DAC (directly connected)
//
//  Advanced control timer
//     TIM1,TIM8
//     On APB2
//     16-bit
//     Similar to GP1
//     Adds repetition rate (RCR) and dead-time detection and insertion
//
// In general all except Basic Timer can be used as an Stm32Timer for its
// basic uses.

// Counter is the timer counter type, uint16_t or uint32_t.

#include <stdint.h>
#include "bits.h"

template <typename Counter>
class Stm32Timer {
    const uint32_t _base;
    const uint32_t _timerclk;
public:
    enum class Register {
        TIM_CR1   = 0x00,
        TIM_CR2   = 0x04,
        TIM_SMCR  = 0x08,
        TIM_DIER  = 0x0c,
        TIM_SR    = 0x10,
        TIM_EGR   = 0x14,
        TIM_CCMR1 = 0x18,
        TIM_CCMR2 = 0x1c,
        TIM_CCER  = 0x20,
        TIM_CNT   = 0x24,
        TIM_PSC   = 0x28,
        TIM_ARR   = 0x2c,
        TIM_CCR1  = 0x34,
        TIM_CCR2  = 0x38,
        TIM_CCR3  = 0x3c,
        TIM_CCR4  = 0x40,
        TIM_DCR   = 0x48,
        TIM_DMAR  = 0x4c,
        // These are specific to these two timers
        TIM2_OR   = 0x50,
        TIM5_OR   = 0x50
    };

    enum {
        // TIM_CR1
        CKD    = 8,
        ARPE   = 7,
        CMS    = 5,
        DIR    = 4,
        OPM    = 3,
        URS    = 2,
        UDIS   = 1,
        CEN    = 0,

        // TIM_SR
        CC4OF = 12,
        CC3OF = 11,
        CC2OF = 10,
        CC1OF = 9,
        TIF   = 6,
        CC4IF = 4,
        CC3IF = 3,
        CC2IF = 2,
        CC1IF = 1,
        UIF   = 0,
        
        // TIM_CR2
        TI1S   = 7,
        MMS    = 4,
        CCDS   = 3,

        // TIM_SMCR
        ETP    = 15,
        ECE    = 14,
        ETPS   = 12,
        ETF    = 8,
        MSM    = 7,
        TS     = 4,
        SMS    = 0,

        // TIM_DIER
        TDE    = 14,
        COMDE  = 13,
        CC4DE  = 12,
        CC3DE  = 11,
        CC2DE  = 10,
        CC1DE  = 9,
        UDE    = 8,
        TIE    = 6,
        CC4IE  = 4,
        CC3IE  = 3,
        CC2IE  = 2,
        CC1IE  = 1,
        UIE    = 0,

        // TIM_EGR
        TG     = 6,
        CC4G   = 4,
        CC3G   = 3,
        CC2G   = 2,
        CC1G   = 1,
        UG     = 0,

        // TIM_CCMR1 (output compare mode)
        OC2CE  = 15,
        OC2M   = 12,
        OC2PE  = 11,
        OC2FE  = 10,
        CC2S   = 8,
        OC1CE  = 7,
        OC1M   = 4,
        OC1PE  = 3,
        OC1FE  = 2,
        CC1S   = 0,

        // TIM_CCMR1 (input compare mode)
        IC2F   = 12,
        IC2PSC = 10,
        //CCS2S  = 8,
        IC1F   = 4,
        IC1PSC = 2,
        //CC1S   = 0,

        // TIM_CCMR2 (output compare mode)

        // TIM_CCMR2 (input compare mode)

        // TIM_CCER
        CC4NP  = 15,
        CC4P   = 13,
        CC4E   = 12,
        CC3NP  = 11,
        CC3P   = 9,
        CC3E   = 8,
        CC2NP  = 7,
        CC2P   = 5,
        C2E    = 4,
        CC1NP  = 3,
        CC1P   = 1,
        CC1E   = 0,

        // TIM_DCR
        DBL    = 8,
        DBA    = 0,

        // TIM2_OR
        ITR1_RMP = 10,

        // TIM5_OR
        IT4_RMP  = 6
    };

    template <typename T>
    T& reg(const Register r) {
        return *((T*)(_base + (uint32_t)r)); 
    }

    Stm32Timer(uint32_t base, uint32_t timerclk)
        : _base(base),
          _timerclk(timerclk)
    { }

    // Run timer once up to count and interrupt.
    void RunTimer(Counter count);

    // Run timer, repeatedly call Tick() at freq Hz
    void RunTimerFreq(uint32_t freq);

    // Read timer count
    uint32_t GetCount() {
        return reg<volatile uint32_t>(Register::TIM_CNT);
    }

    // Interrupt handler
    static void Interrupt(void* token);

    // Called from interrupt handler to indicate clock tick
    virtual void Tick() = 0;

private:
    [[__finline]] inline void HandleInterrupt() {
        volatile uint32_t& sr = reg<volatile uint32_t>(Register::TIM_SR);
        if (sr & BIT(UIF)) {
            Tick();
            sr &= ~BIT(UIF);
        }
    }
        
private:
    Stm32Timer();
};


// System clock
class Clock: public Stm32Timer<uint32_t> {
	uint64_t _time;
public:
	Clock(uint32_t base, uint32_t timerclk)
        : Stm32Timer<uint32_t>(base, timerclk),
          _time(0)
    { }

	uint64_t GetTime() { return _time + GetCount(); }

	void Tick();
};

#endif // __STM32_TIMER_H__
