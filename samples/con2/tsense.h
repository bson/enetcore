#ifndef __TSENSE_H__
#define __TSENSE_H__

#include "enetkit.h"
#include "sampler.h"

class TSense: public Sampler<Timer16, 10, 16, Adc::Trigger::TIM3_TRGO, APB1_TIMERCLK> {
    uint32_t _value;
    bool _ready = false;
public:
    TSense()
        : Sampler(BASE_ADC1, BASE_TIM3) {
    };

    void Run(uint freq) {
        Sampler::Run(freq);
    }

    void SampleReady(uint32_t value) {
        _ready = true;
        _value = value;
        Thread::WakeSingle(this);
    }

    bool Ready() const { return _ready; }
    uint32_t Value() {
        Thread::IPL G(IPL_ADC);
        _ready = false;
        return _value;
    }

    float Temp() {
        return (float)Value() / 100.0; // XXX implement me
    }
};

#endif // __TSENSE_H__
