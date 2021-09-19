// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "util.h"

//extern Eeprom _eeprom;

extern PinNegOutput<Gpio::Pin> _led;

int main() {
    DMSG("Main: blinking lights");

    Time wake = Time::Now();
    for (;;) {
        _led.Raise();
        wake += Time::FromMsec(500);
        Thread::Sleep(wake);

        _led.Lower();
        wake += Time::FromMsec(250);
        Thread::Sleep(wake);
    }
}
