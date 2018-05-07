// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetcore.h"
#include "tsc2046.h"
#include "board.h"

namespace tsc2046 {

void TouchController::Init() {
    Mutex::Scoped L(_lock);

    _spi.Init();
    _spi.Configure(0, BUS_SPEED);

#if 0
    _spi.Select();
    _spi.Send(START | PD_ENABLE);
    _spi.Deselect();
#endif
}


bool TouchController::ReadPosition(uint16_t& x, uint16_t& y) {
    Mutex::Scoped L(_lock);
    LpcSpiDev::AcquireBus G(_spi);

    _spi.Select();

    _spi.Send(START | MODE_12BIT | DIFFERENTIAL | MEASURE_X | PD_REFON_ADCON);

    Thread::Delay(5);

    uint16_t value_x = _spi.Read() << 8;
    value_x |= _spi.SendRead(START | MODE_12BIT | DIFFERENTIAL | MEASURE_Y
                             | PD_REFON_ADCON);

    Thread::Delay(5);

    uint16_t value_y = _spi.Read() << 8;
    value_y |= _spi.SendRead(START | PD_ENABLE);
    
    _spi.Deselect();

    x = value_x >> 4;
    y = value_y >> 4;

    return true;
}

}; // ns tsc2046
