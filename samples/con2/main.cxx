// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "util.h"
#include "tsense.h"

TSense _tsense;

extern PinOutput<Gpio::Pin> _led;

#ifdef ENABLE_PANEL
Thread* _ui_thread;
#endif

template <typename T>
static T abs(const T& a) {
    return a < (T)0 ? -a : a;
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
         _tsense.Value()/(TSENSE_NSAMPLES),
         _tsense.Count());
}

int main() {
    Thread::EnableFP();

#ifdef ENABLE_PANEL
    extern void* UIThread(void*);
    _ui_thread = Thread::Create("ui", UIThread, NULL, UI_THREAD_STACK);
#endif

    _tsense.SetUnit(_tsense.Unit::F);
    _tsense.Run(8);

    DMSG("Main: blinking lights");

    Time wake = Time::Now();
    for (;;) {
        _led.Raise();
        logDateTime();
        wake += Time::FromMsec(500);
        Thread::Sleep(wake);

        _led.Lower();
        logDateTime();
        wake += Time::FromMsec(250);
        Thread::Sleep(wake);
    }
}
