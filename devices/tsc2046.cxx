// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifdef ENABLE_TSC2046

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
    LpcSpiDev::AcquireBus G(_spi);

    _spi.Select();
    _spi.Send(SAMPLE_X);
    const int vx1 = _spi.SendRead(0);
    const int vx2 = _spi.SendRead(SAMPLE_Y);
    const int vy1 = _spi.SendRead(0);
    const int vy2 = _spi.SendRead(IDLE);
    _spi.Deselect();

    x = max<int>((vx1 << 4) | (vx2 >> 4), MIN_X) - MIN_X;
    y = max<int>((vy1 << 4) | (vy2 >> 4), MIN_Y) - MIN_Y;
    x = MAX_X - min<int>(x, MAX_X);
    y = MAX_Y - min<int>(y, MAX_Y);

    x = uint(x) * _width / MAX_X;
    y = uint(y) * _height / MAX_Y;

#if 0
    DMSG("X1: %x", vx1);
    DMSG("X2: %x", vx2);
    DMSG("Y1: %x", vy1);
    DMSG("Y2: %x", vy2);
#endif

    return vx1 != -1;
}

}; // ns tsc2046

#endif // ENABLE_TSC2046
