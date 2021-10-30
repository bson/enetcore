#ifndef __SAMPLER_H__
#define __SAMPLER_H__

#include "core/enetkit.h"
#include "core/thread.h"
#include "core/ring.h"


// Simple sampler with boxcar filtering
template <typename Timer, uint32_t NSAMPLES, uint32_t RING_SIZE,
          uint16_t INPUT, Adc::Trigger TRIGGER, uint32_t TIMER_APB_TCLK>
class Sampler: public Adc {
    static_assert(RING_SIZE >= NSAMPLES + 2);
    static_assert((RING_SIZE & (RING_SIZE-1)) == 0); // Must be power of 2

    Timer _timer;
    Ring<RING_SIZE, uint16_t> _samples;
    uint32_t _sum = 0;
    uint16_t _ovr_count = 0;

public:
    Sampler(uint32_t adc_base, uint32_t timer_base)
        : Adc(adc_base),
          _timer(timer_base, TIMER_APB_TCLK) {
    }

    void Run(uint32_t freq) {
        _timer.RunTimerFreq(freq, TIMER_APB_TCLK/0x10000, false, true);
        Adc::Configure(INPUT, TRIGGER, Adc::Mode::SINGLE, Adc::SampleTime::TS_480);
    }
        
    uint16_t Overruns() const { return _ovr_count; }

    uint32_t GetSample() const {
        return _sum;
    }

    // * implements Adc::AdcComplete
    void AdcComplete(uint16_t sample, bool ovr) {
        if (ovr)
            ++_ovr_count;

        _samples.PushBack(sample);
        _sum += sample;
        if (_samples.Size() > NSAMPLES) {
            _sum -= _samples.PopFront();
            SampleReady();
        }
    }

    virtual void SampleReady() = 0;
};

#endif // __SAMPLER_H__
