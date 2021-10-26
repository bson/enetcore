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
    float _temp = 0.0f;
    uint32_t _count = 0;
    bool _ready = false;

    // Murata NCP21XW153J03RA
    // NTC 15k 3%
    // T0 = 25 deg C (298.15K)
    // R = 15k
    // R0 = 1k
    // B  = 3950

    enum {
        T0   = 25,
        B    = 3950,
        S_FS = (1 << 12)       // Full scale sample value
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

    float Temp() {
        if (_ready) {
            float s = float(Value()) / float(TSENSE_NSAMPLES);

            // 1/T = 1/T0 + 1/B * ln(R/R0)
            // R = R0 * ( ( Sfs / S ) - 1 )
            // => 1/T = 1/T0 + 1/B * ln( R0 * ( ( Sfs / S ) - 1 ) / R0 )
            // <=> 1/T = 1/T0 + 1/B * ln( ( Sfs / S ) – 1 )
            _temp = 1.0f/(1.0f/(float(T0)+273.15f) + 1.0f/float(B) * logf(float(S_FS)/s - 1.0f)) - 273.15f;
        }
        return _temp;
    }
};

#endif // __TSENSE_H__
