// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetkit.h"
#include "core/util.h"
#include "tsense.h"
#include "audio/audio.h"

TSense _tsense;

extern PinOutput<Gpio::Pin> _led;

#ifdef ENABLE_PANEL
Thread* _ui_thread;
#endif

namespace sound {
extern const Sound bell;
}

template <typename T>
static T abs(const T& a) {
    return a < (T)0 ? -a : a;
}


static void play(const Sound& sound) {
    _tim6.RunTimerFreq(sound.samplerate, APB1_TIMERCLK/0x10000, false, true);
    _dac.Output(sound.samples, sound.nsamples, Dac::Trigger::TIM6_TRGO,
                _dma1, DMA_STREAM_DAC, DMA_CHANNEL_DAC, DMA_PRIORITY_DAC);
}

static void logDateTime() {
    const auto now = Rtc::GetDateTime();
    static const char* const days[] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    const int temp = _tsense.Temp() * 100.0f;

    DMSG("%s %04d-%02d-%02d %02d:%02d:%02d  Tsense: %d.%02uF %u (%d)",
         days[now.dow-1],
         now.year,
         now.month,
         now.day,
         now.hour,
         now.min,
         now.sec,
         temp/100, abs(temp) % 100,
         _tsense.Value(),
         _tsense.Count());
}

int main() {
    Thread::EnableFP();

#ifdef ENABLE_PANEL
    void* UIThread(void*);
//    _ui_thread = Thread::Create("ui", UIThread, NULL, UI_THREAD_STACK);
#endif

    _tsense.SetUnit(_tsense.Unit::F);
    _tsense.Run(8);

    play(sound::bell);

    DMSG("Main: blinking lights");

    bool ssr = false;
    int ssr_count = 0;

    Time wake = Time::Now();
    for (;;) {
        if (++ssr_count >= 10) {
            extern PinNegOutput<Gpio::Pin> _ssr_conduct;

            ssr_count = 0;
            if (ssr)
                _ssr_conduct.Raise();
            else
                _ssr_conduct.Lower();

            ssr = !ssr;
        }

        _led.Raise();
        logDateTime();
        wake += Time::FromMsec(150);
        Thread::Sleep(wake);

        _led.Lower();
        logDateTime();
        wake += Time::FromMsec(500);
        Thread::Sleep(wake);
    }
}
