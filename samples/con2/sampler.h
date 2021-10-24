#ifndef __SAMPLER_H__
#define __SAMPLER_H__

#include "enetkit.h"
#include "thread.h"
#include "ring.h"


// Simple sampler with boxcar filtering
template <typename Timer, uint32_t NSAMPLES, uint32_t RING_SIZE,
          Adc::Trigger TIMER_TRIGGER, uint32_t TIMER_APB_TCLK>
class Sampler: public Adc {
    static_assert(RING_SIZE >= NSAMPLES + 2);
    static_assert((RING_SIZE & (RING_SIZE-1)) == 0); // Must be power of 2

    Timer _timer;
    Ring<RING_SIZE> _samples;
    uint32_t _sum = 0;
    uint32_t _ovr_count = 0;

public:
    Sampler(uint32_t adc_base, uint32_t timer_base)
        : Adc(adc_base),
          _timer(timer_base, TIMER_APB_TCLK) {
    }

    void Run(uint32_t freq) {
        _timer.RunTimerFreq(freq, 100); // 1/100 = 1% precision
        Adc::Configure(0, TIMER_TRIGGER, Adc::Mode::CONT, Adc::SampleTime::TS_480);
    }
        
    void AdcComplete(bool ovr) {
        if (ovr)
            ++_ovr_count;

        const uint32_t sample = Adc::GetSample();
        _samples.PushBack(sample);
        _sum += sample;
        if (_samples.Size() > NSAMPLES) {
            _sum -= _samples.PopFront();
            SampleReady(_sum);
        }
    }

    virtual void SampleReady(uint32_t value) = 0;
};

#endif // __SAMPLER_H__
