// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetcore.h"
#include "devices/tsc2046.h"
#include "board.h"

namespace tsc2046 {

void TouchController::Init() {
    Mutex::Scoped L(_lock);

    enum {
        IDLE = START | PD_REFON_ADCOFF
    };

    _spi.Init();
    _spi.Configure(0, BUS_SPEED);

    _spi.Select();
    _spi.Send(IDLE);
    _spi.Deselect();
}


bool TouchController::ReadPosition(uint16_t& x, uint16_t& y) {
    enum {
        IDLE = START | PD_REFON_ADCOFF,
        SAMPLE_X = START | MODE_12BIT | DIFFERENTIAL | MEASURE_X | PD_REFON_ADCON,
        SAMPLE_Y = START | MODE_12BIT | DIFFERENTIAL | MEASURE_Y | PD_REFON_ADCON
    };

    Mutex::Scoped L(_lock);

    _spi.Acquire();
    _spi.Select();

    static const uint8_t tx[] = {SAMPLE_X, 0, SAMPLE_Y, IDLE};
    static uint8_t rx[4];

    _spi.Transact(tx, sizeof tx, rx, sizeof rx);
    Thread::WaitFor(&_spi);

    const int vx1 = rx[0];
    const int vx2 = rx[1];
    const int vy1 = rx[2];
    const int vy2 = rx[3];

    _spi.Deselect();
    _spi.Release();

    x = max<int>((vx1 << 4) | (vx2 >> 4), MIN_X) - MIN_X;
    y = max<int>((vy1 << 4) | (vy2 >> 4), MIN_Y) - MIN_Y;
    x = MAX_X - min<int>(x, MAX_X);
    y = MAX_Y - min<int>(y, MAX_Y);

    x = uint(x) * _width / MAX_X;
    y = uint(y) * _height / MAX_Y;

    return vx1 != -1;
}

}; // ns tsc2046
