// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __TSENSE_H__
#define __TSENSE_H__

#include "enetkit.h"
#include "sampler.h"
#include "nvic.h"
#include "float.h"

enum { TSENSE_NSAMPLES = 10 };

[[__optimize]]
static float logf(float x) {
    float v = (x-1.0f)/(x+1.0f);
    float div = 1.0f;
    float sum = v;
    const float vsq = v*v;

    for (uint i = 0; i < 10; i++) {
        v *= vsq;
        div += 2.0f;
        sum += v/div;
    }

    return sum*2.0f;
}

class TSense: public Sampler<Timer16, TSENSE_NSAMPLES, 16, 8, Adc::Trigger::TIM3_TRGO, APB1_TIMERCLK> {
public:
    enum class Unit {
        F, C, K
    };

private:
    float _temp     = 0.0f;
    uint32_t _count = 0;
    bool _ready     = false;
    Unit _unit      = Unit::F;

    // Murata NCP21XW153J03RA
    // NTC 15k 3%
    // T0 = 25 deg C (298.15K)
    // R = 15k
    // R0 = 1k
    // B  = 3950

    enum {
        T0   = 25,
        B    = 3950,
        RT0  = 15000,
        R0   = 1000,
        S_FS = (1 << 12) - 1    // Full scale sample value
    };

public:
    TSense()
        : Sampler(BASE_ADC1, BASE_TIM3) {
    };

    void Run(uint freq) {
        NVic::InstallIRQHandler(INTR_ADC, Adc::Interrupt, IPL_ADC, this);
        NVic::EnableIRQ(INTR_ADC);
        NVic::DisableIRQ(INTR_TIM3);

        Sampler::Run(freq);
    }

    // * implements Sampler::SampleReady
    void SampleReady() {
        _ready = true;
        ++_count;
        Thread::WakeSingle(this);
    }

    bool Ready() const {
        return _ready;
    }

    uint32_t Value() {
        Thread::IPL G(IPL_ADC);
        _ready = false;
        return GetSample();
    }

    uint32_t Count() const {
        return _count;
    }

    // Return temp in C
    float Temp() {
        if (_ready)
            recalcTemp();

        return _temp;
    }

    void SetUnit(Unit u) {
        if (exch(_unit, u) != u && _count)
            recalcTemp();
    }

private:
    void recalcTemp() {
        float s;
        {
            Thread::IPL G(IPL_ADC);
            //s = float(Value()) / float(TSENSE_NSAMPLES);
            s = float(Value());
        }
        // In a divider with a constant R0 on top and the thermistor resistance R on the
        // bottom half,
        //     Vout = Vin (R0/(R + R0))
        // =>
        //     R = R0 ((Vin - Vout)/Vout)
        //     R = R0 ((s - FS))/s)
        //
        // with a thermistor resistance RT0 at T0,
        //
        //     B = Log[(R/RT0)/(1/T–1/T0)]
        //
        // Substituting R from above and solving for T gives:
        //
        // =>  T = (B T0)/(B + T0 Log[(R0 (FS - s))/(RT0 s)])
        //
        // T0 and T are in Kelvin.
        // s is the sample, FS the full scale sample value.
        //
        const float t0 = T0 + 273.15f;
        const float b = B;
        const float bt0 = B * T0;
        const float r0 = R0;
        const float rt0 = RT0;
        const float fs = S_FS;

        const float tempk = bt0/(b + t0 * logf((r0 * (fs - s))/(rt0 * s)));
        switch (_unit) {
        case Unit::C:
            _temp = tempk - 273.15f;
            break;
        case Unit::F:
            _temp = tempk * 9.0f/5.0f - 459.67;
            break;
        case Unit::K:
            _temp = tempk;
        }
    }
};

#endif // __TSENSE_H__
