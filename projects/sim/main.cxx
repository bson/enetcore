// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetkit.h"
#include "core/util.h"

extern PinOutput<Gpio::Pin> _card_led;

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

    DMSG("%s %04d-%02d-%02d %02d:%02d:%02d\n",
         days[now.dow-1],
         now.year,
         now.month,
         now.day,
         now.hour,
         now.min,
         now.sec);
}

int main() {
    Thread::EnableFP();

    DMSG("Main: blinking lights");

    bool ssr = false;
    int ssr_count = 0;

    Time wake = Time::Now();
    for (;;) {
        _card_led.Raise();
        logDateTime();
        wake += Time::FromMsec(150);
        Thread::Sleep(wake);

        _card_led.Lower();
        logDateTime();
        wake += Time::FromMsec(500);
        Thread::Sleep(wake);
    }
}
