#ifndef __ADC_H__
#define __ADC_H__

#include "thread.h"

// The ADC is really 3 ADCs in a single peripheral with a common
// block.  We split them into three separate logical peripherals used
// independently.  For multi-converter setups the common block should
// be implemented, possibly as a supervisor.

class Stm32Adc {
    enum class Register {
        SR = 0x00,
        CR1 = 0x04,
        CR2 = 0x08,
        SMPR1 = 0x0c,
        SMPR2 = 0x10,
        JOFR1 = 0x14,
        JOFR2 = 0x18,
        JOFR3 = 0x1c,
        JOFR4 = 0x20,
        HTR = 0x24,
        LTR = 0x28,
        SQR1 = 0x2c,
        SQR2 = 0x30,
        SQR3 = 0x34,
        JSQR = 0x38,
        JDR1 = 0x3c,
        JDR2 = 0x40,
        JDR3 = 0x44,
        JDR4 = 0x48,
        DR = 0x4c
    };

    enum {
        // SR
        OVR = 5,
        STRT = 4,
        JSTRT = 3,
        JEOC = 2,
        EOC = 1,
        AWD = 1,

        // CR1
        OVRE = 26,
        RES = 24,
        AWDEN = 23,
        JAWDEN = 22,
        DISCNUM = 13,
        JDISCEN = 12,
        DISCEN = 11,
        JAUTO = 10,
        AWSSGL = 9,
        SCAN = 8,
        JEOCIE = 7,
        AWDIE = 6,
        EOCIE = 5,

        // CR2
        SWSTART = 30,
        EXTEN = 28,
        EXTSEL = 24,
        JWSSTART = 22,
        JEXTEN = 20,
        JEXTSEL = 16,
        ALIGN = 11,
        EOCS = 10,
        DDS = 9,
        DMA = 8,
        CONT = 1,
        ADON = 0,

        // SQR1
        L = 20,

        // JSQR
        JL = 20,
    };

    const uint32_t _base;

    volatile uint32_t& reg(Register r) {
        return *(volatile uint32_t*)(_base + (uint32_t)r);
    }

public:
     enum class Trigger {
        TIM1_CC1  = 0b0000,
        TIM1_CC2  = 0b0001,
        TIM1_CC3  = 0b0010,
        TIM2_CC2  = 0b0011,
        TIM2_CC3  = 0b0100,
        TIM2_CC4  = 0b0101,
        TIM2_TRGO = 0b0110,
        TIM3_CC1  = 0b0111,
        TIM3_TRGO = 0b1000,
        TIM4_CC4  = 0b1001,
        TIM5_CC1  = 0b1010,
        TIM5_CC2  = 0b1011,
        TIM5_CC3  = 0b1100,
        TIM8_CC1  = 0b1101,
        TIM8_TRGO = 0b1110,
        EXTI11    = 0b1111,
        NONE      = 0b10000,
    };

    enum class Mode {
        CONT   = BIT(CONT),
        SINGLE = 0
    };

    // Sample time, in ADCCLK cycles
    enum class SampleTime {
        TS_3   = 0,
        TS_15  = 1,
        TS_28  = 2,
        TS_56  = 3,
        TS_84  = 4,
        TS_112 = 5,
        TS_144 = 6,
        TS_480 = 7
    };

    Stm32Adc(uint32_t base)
        : _base(base) {
    }

    void Configure(uint32_t ch, Trigger t, Mode m, SampleTime ts) {
        assert(ch <= 18);

        Thread::IPL G(IPL_ADC);

        reg(Register::CR2) |= BIT(ADON);

        uint32_t tbits = 0;
        if (t < Trigger::NONE)
            tbits = (1 << EXTEN) | ((uint32_t)t << EXTSEL);

        reg(Register::CR2) = tbits | BIT(EOCS) | (uint32_t)m | BIT(ADON);

        volatile uint32_t& smpr = reg(ch >= 10 ? Register::SMPR1 : Register::SMPR2);
        const uint32_t bits = ch >= 10 ? ch - 10 : ch;
        smpr = (smpr & ~(7 << bits)) | ((uint32_t)ts << bits);
    
        reg(Register::SR) &= ~BIT(OVR);
        reg(Register::CR1) |= BIT(EOCIE);
        reg(Register::SQR1) = 0b01 << L;
        reg(Register::SQR3) = ch;
    }

    void Start() {
        volatile uint32_t& cr2 = reg(Register::CR2);
        cr2 |= SWSTART;
    }

    uint16_t GetSample() {
        return reg(Register::DR);
    }

    virtual void AdcComplete(bool ovr);

    void HandleInterrupt();

    static void Interrupt(void *token);
};


class Stm32AdcCommon {
    enum class Register {
        CSR = 0x00,
        CCR = 0x04,
        CDR = 0x08
    };

    enum { 
        // CCR
        TSVREFE = 23,
        VBATE = 22,
        ADCPRE = 16
    };

    static volatile uint32_t& reg(Register r) {
        return *(volatile uint32_t*)(BASE_ADCCOM + (uint32_t)r);
    }

public:
    static void SetPrescaler(uint32_t psc) {
        assert(psc <= 3);
        reg(Register::CCR) = (reg(Register::CCR) & ~(3 << ADCPRE)) | (psc << ADCPRE);
    }
    static void EnableVbat() {
        reg(Register::CCR) |= BIT(VBATE);
    }
    static void EnableTemp() {
        reg(Register::CCR) |= BIT(TSVREFE);
    }
};

#endif // __ADC_H__
