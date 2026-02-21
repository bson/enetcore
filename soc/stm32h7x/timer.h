// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_TIMER_H__
#define __STM32_TIMER_H__


// There are four kinds of timers:
//
//  AC Advanced Control
//     TIM1,TIM8
//     On APB2 (XXX)
//     16-bit
//     Similar to GP1
//     Adds repetition rate (RCR) and dead-time detection and insertion
//
//  GP1 General purpose 1
//     TIM2/3/4/5
//     TIM2 and 5 are 32-bit
//     4 CCRs
//     These are all on APB1
//     Count up, down, or up-down
//     Supports quadrature encoding
//
//  GP2 General purpose 2
//     TIM12/13/14
//     All 16-bit
//     2 CCRs
//     TIM9-11 on APB2 (XXX)
//     TIM12-14 on APB1 (XXX)
//
//  GP3 General purpose 3
//     TIM16/17
//     All 16-bit
//     2 CCRs
//     TIM9-11 on APB2
//     TIM12-14 on APB1
//
//  GP4 General Purpose 4
//     TIM15
//     All 16 bit
//
//  BT Basic
//     TIM6,7
//     16-bit
//     Up count only, reload, ARR
//     No CCRs
//     On APB1
//     Mainly intended to drive DAC (directly connected)
//
// In general all except Basic Timer can be used as an Stm32Timer for its
// basic uses.

// Counter is the timer counter type, uint16_t or uint32_t.

#include <stdint.h>
#include "core/bits.h"

template <typename Counter>
class Stm32Timer {
    const uint32_t _base;
    const uint32_t _timerclk;
public:
    enum class Register {
        TIM_CR1   = 0x00,       // All
        TIM_CR2   = 0x04,       // AC, GP1, GP3, GP4, BT
        TIM_SMCR  = 0x08,       // AC, GP1, GP4
        TIM_DIER  = 0x0c,       // All
        TIM_SR    = 0x10,       // All
        TIM_EGR   = 0x14,       // All
        TIM_CCMR1 = 0x18,       // AC, GP1, GP2, GP3, GP4
        TIM_CCMR2 = 0x1c,       // AC, GP1
        TIM_CCER  = 0x20,       // AC, GP1, GP2, GP3, GP4
        TIM_CNT   = 0x24,       // All
        TIM_PSC   = 0x28,       // All
        TIM_ARR   = 0x2c,       // All
        TIM_RCR   = 0x30,       // AC, GP3, GP4
        TIM_CCR1  = 0x34,       // AC, GP1, GP2, GP3, GP4
        TIM_CCR2  = 0x38,       // AC, GP1, GP4
        TIM_CCR3  = 0x3c,       // AC, GP1
        TIM_CCR4  = 0x40,       // AC, GP1
        TIM_BDTR  = 0x44,       // AC, GP3, GP4
        TIM_DCR   = 0x48,       // AC, GP1, GP3, GP4
        TIM_DMAR  = 0x4c,       // AC, GP1, GP3, GP4
        TIM_CCMR3 = 0x54,       // AC
        TIM_CCR5  = 0x58,       // AC
        TIM_CCR6  = 0x5c,       // AC
        TIM_AF1   = 0x60,       // AC, GP1, GP3, GP4
        TIM_AF2   = 0x64,       // AC
        TIM_TISEL = 0x68,       // AC, GP1, GP2, GP3, GP4
    };

    enum {
        // TIM_CR1
        UIFREMAP = 11,
        CKD    = 8,
        ARPE   = 7,
        CMS    = 5,
        DIR    = 4,
        OPM    = 3,
        URS    = 2,
        UDIS   = 1,
        CEN    = 0,

        // TIM_CR2
        MMS2   = 20,
        OIS6   = 18,
        OIS5   = 16,
        OIS4   = 14,
        OIS3   = 12,
        OIS2N  = 11,
        OIS2   = 10,
        OIS1N  = 9,
        OIS1   = 8,
        TI1S   = 7,
        MMS    = 4,
        CCDS   = 3,
        CCUS   = 2,
        CCPC   = 1,

        // TIM_SMCR
        TS2    = 20,
        SMS2   = 16,
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
        BIE    = 7,
        TIE    = 6,
        COMIE  = 5,
        CC4IE  = 4,
        CC3IE  = 3,
        CC2IE  = 2,
        CC1IE  = 1,
        UIE    = 0,

        // TIM_SR
        CC6IF  = 17,
        CC5IF  = 16,
        SBIF   = 13,
        CC4OF  = 12,
        CC3OF  = 11,
        CC2OF  = 10,
        CC1OF  = 9,
        B2IF   = 8,
        BIF    = 7,
        TIF    = 6,
        CC4IF  = 4,
        CC3IF  = 3,
        CC2IF  = 2,
        CC1IF  = 1,
        UIF    = 0,

        // TIM_EGR
        B2G    = 8,
        BG     = 7,
        TG     = 6,
        COMG   = 5,
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

        // TIM_CCMR1 (input capture mode)
        IC2F   = 12,
        IC2PSC = 10,
        CCS2S  = 8,
        IC1F   = 4,
        IC1PSC = 2,
        CC1S   = 0,

        // TIM_CCMR2 (input capture mode)
        IC4F   = 12,
        IC4PSC = 10,
        CC4S   = 8,
        IC3F   = 3,
        IC3PSC = 2,
        CC3S   = 0,

        // TIM_CCMR2 (output compare mode)
        CC6P   = 21,
        CC6E   = 20,
        CC5P   = 17,
        CC5E   = 16,
        CC4NP  = 15,
        CC4P   = 13,
        CC4E   = 12,
        CC3NP  = 11,
        CC3NE  = 10,
        CC3P   = 9,
        CC3E   = 8,
        CC2NP  = 7,
        CC2NE  = 6,
        CC2P   = 5,
        CC2E   = 4,
        CC1NP  = 3,
        CC1NE  = 2,
        CC1P   = 1,
        CC1E   = 0,

        // TIM1, TIM8 (AC)  TIM_CCMR3 (output compare mode only)
        OC6CE  = 15,
        OC6M2  = 24,
        OC6M   = 12,
        OC6PE  = 11,
        OC6FE  = 10,
        O5CE   = 7,
        OC5M2  = 16,
        OC5M   = 4,
        OC5PE  = 3,
        OC5FE  = 2,

        // TIM_CCR5
        GC5C3  = 31,
        GC5C2  = 30,
        GC5C1  = 29,
        CCR    = 0,

        // TIM_AF1
        // Not all timers have all bits/fields
        ETRSEL    = 14,
        BKCMP2P   = 11,
        BKCMP1P   = 10,
        BKINP     = 9,
        BKDF1BK0E = 8,
        BKCMP2E   = 2,
        BKCMP1E   = 1,
        BKINE     = 0,

        // TIM_AF2
        // TIM1, TIM8 only
        BK2CMP2P   = 11,
        BK2CMP1P   = 10,
        BK2INP     = 9,
        BK2DF1BK1E = 8,
        BK2CMP2E   = 2,
        BK2CMP1E   = 1,
        BK2INE     = 0,

        // TIM_CCER
        // Subset of TIM_CCMR2

        // TIM_BDTR
        BK2P   = 25,
        BK2E   = 24,
        BK2F   = 20,
        BKF    = 16,
        MOE    = 15,
        AOE    = 14,
        BKP    = 13,
        BKE    = 12,
        OSSR   = 11,
        OSSI   = 10,
        LOCK   = 8,
        DTG    = 0,

        // TIM_DCR
        DBL    = 8,
        DBA    = 0,
    };

    enum {
        PWM_PREC = 1024,
        PWM_OFF  = PWM_PREC + 1 // Duty value to shut off
    };

    enum class PwmPolarity {
        HIGH = BIT(CC1P),
        LOW = 0,
    };

    enum class CCR {
        CCR1 = 0, CCR2, CCR3, CCR4
    };

    enum class PwmMode {
        MODE1 = 6,
        MODE2 = 7
    };

    volatile uint32_t& reg(const Register r) {
        return *((volatile uint32_t*)(_base + (uint32_t)r)); 
    }

    volatile uint32_t& r_ccr(CCR ccr) {
        return *((volatile uint32_t*)(_base + (uint32_t)Register::TIM_CCR1
                                      + (uint32_t)ccr * 4));
    }

    Stm32Timer(uint32_t base, uint32_t timerclk)
        : _base(base),
          _timerclk(timerclk)
    { }

    // Run timer, repeatedly call Tick() in an interrupt context at
    // IPL_TIMER at freq Hz.  Count with precision 'timebase'.  If
    // intr is true, enable interrupts to call Tick().  If trgo is
    // true, generate trigger out on update (full count).
    void RunTimerFreq(uint32_t freq, uint32_t timebase, bool intr = true,
                      bool trgo = false);

    // Run timer in PWM mode.  The CCRs are used to generate outputs.
    void RunPwm(uint32_t freq);

    // Enable PWM output for a CCR.
    void ConfigurePwm(CCR ccr, PwmPolarity pol);

    // Use a CCR previously set up with ConfigurePwm to output a PWM
    // signal with a duty.  Duty is in units of 1/PWM_PREC and the
    // actual duty is (duty/PWM_PREC+1) * 100%.  This means 0
    // represents a brief pulse of 1/PWM_PREC, or ~0.1% of the period.
    // To completely shut off the output, set the duty to a value >
    // PWM_PREC; this will cause the CCR to never match the count.
    void SetPwmDuty(CCR ccr, uint32_t duty) {
        assert(duty <= PWM_OFF);
        r_ccr(ccr) = duty * reg(Register::TIM_ARR) / PWM_PREC;
    }

    // Read timer count
    uint32_t GetCount() {
        return reg(Register::TIM_CNT);
    }

    // Interrupt handler
    static void Interrupt(void* token);

    // Called from interrupt handler to indicate clock tick
    virtual void Tick() { };

protected:
    void HandleInterrupt();
        
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

    void Start();

	uint64_t GetTime();

	void Tick();
};

#endif // __STM32_TIMER_H__
