// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "util.h"

extern PinOutput<Gpio::Pin> _led;

#ifdef ENABLE_ENET
Thread* _net_thread;
#endif
#ifdef ENABLE_PANEL
Thread* _ui_thread;
#endif

extern void UsbInit();
extern void* UIThread(void*);

void logDateTime() {
    const auto now = Rtc::GetDateTime();
    static const char* const days[] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    DMSG("%s %04d-%02d-%02d %02d:%02d:%02d",
         days[now.dow-1],
         now.year,
         now.month,
         now.day,
         now.hour,
         now.min,
         now.sec);
}

int main() {
#ifdef ENABLE_USB
    UsbInit();
#endif

#ifdef ENABLE_ENET
    _net_thread = Thread::Create("network", NetThread, NULL, NET_THREAD_STACK);
#endif

#ifdef ENABLE_PANEL
    _ui_thread = Thread::Create("ui", UIThread, NULL, UI_THREAD_STACK);
#endif

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
