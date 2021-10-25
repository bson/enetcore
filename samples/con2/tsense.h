#ifndef __TSENSE_H__
#define __TSENSE_H__

#include "enetkit.h"
#include "sampler.h"
#include "nvic.h"


class TSense: public Sampler<Timer16, 10, 16, 8, Adc::Trigger::TIM3_TRGO, APB1_TIMERCLK> {
    uint32_t _count = 0;
    bool _ready = false;
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
        return (float)Value() / 100.0; // XXX implement me
    }
};

#endif // __TSENSE_H__
