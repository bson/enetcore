// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetcore.h"
#include "devices/tsc2046.h"
#include "core/arithmetic.h"
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

    static const uint8_t tx[] = {SAMPLE_X, 0, SAMPLE_Y, 0, IDLE};
    uint16_t rx[] = { 0, 0, 0 };

    static_assert(sizeof rx - 1 >= sizeof tx);

    _spi.Transact(tx, sizeof tx, (uint8_t*)rx + 1, sizeof tx);
    Thread::WaitFor(&_spi);

    _spi.Deselect();
    _spi.Release();

    if (!rx[1])
        return false;

    int vx = (BE16(rx[1]) & 0x7fff) >> 4;
    int vy = (BE16(rx[2]) & 0x7fff) >> 4;

    vx = max<int>(vx, MIN_X) - MIN_X;
    vy = max<int>(vy, MIN_Y) - MIN_Y;
    vx = MAX_X - min<int>(vx, MAX_X);
    vy = MAX_Y - min<int>(vy, MAX_Y);

    x = uint(vx) * _width / MAX_X;
    y = uint(vy) * _height / MAX_Y;

    return true;
}

}; // ns tsc2046
