#ifndef __TSENSE_H__
#define __TSENSE_H__

#include "enetkit.h"
#include "sampler.h"
#include "nvic.h"
#include "float.h"


static float logf(float x) {
    float v = (x-1.0f)/(x+1.0f);
    float div = 1.0f;
    float sum = v;

    for (uint i = 0; i < 5; i++) {
        v *= v;
        div += 2.0f;
        sum += v/div;
    }

    return sum*2.0f;
}

class TSense: public Sampler<Timer16, 10, 16, 8, Adc::Trigger::TIM3_TRGO, APB1_TIMERCLK> {
    uint32_t _count = 0;
    bool _ready = false;

    // Murata NCP21XW153J03RA
    // NTC 15k 3%
    // T0 = 25 deg C
    // R0 = 15k
    // B  = 3950
    //
    // 1/T = 1/T0 + 1/B * ln(R/R0)
    // R = R0 * ( ( Sfs / S ) - 1 )
    // => 1/T = 1/T0 + 1/B * ln( R0 * ( ( Sfs / S ) - 1 ) / R0 )
    // <=> 1/T = 1/T0 + 1/B * ln( ( Sfs / S ) – 1 )

    enum {
        T0   = 25,
        B    = 3950,
        S_FS = (1 << 10) - 1 // Full scale sample value
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
        return 1.0f/(1.0f/float(T0) + 1.0f/float(B) +
                           logf(float(S_FS)/float(Value()) - 1.0f));
    }
};

#endif // __TSENSE_H__
