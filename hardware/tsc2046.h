// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _TSC2046_H_
#define _TSC2046_H_

#include "mutex.h"

namespace tsc2046 {

class TouchController {
    SpiDev& _spi;
    mutable Mutex _lock;

    const uint _height;
    const uint _width;

    enum { BUS_SPEED = 1000000 }; // 1MHz

    enum {
        MAX_X = 1780,
        MIN_X = 135,
        MAX_Y = 1540,
        MIN_Y = 195,
    };

    enum {
        START = BIT7,
        A_FLD = BIT4,
        MEASURE_X = 0b101 * A_FLD,
        MEASURE_Y = 0b001 * A_FLD,
        MODE_8BIT = BIT3,
        MODE_12BIT = 0,
        SINGLE_ENDED = BIT2,
        DIFFERENTIAL = 0,
        PD0 = BIT0,
        PD1 = BIT1,
        PD_ENABLE = 0,             // Power down between conversions, IRQ enable
        PD_REFOFF_ADCON = PD0,     // Ref off, ADC on, IRQ disable
        PD_REFON_ADCOFF = PD1,     // Ref on, ADC off, IRQ enable
        PD_REFON_ADCON  = PD0|PD1, // Ref on, ADC on, IEQ disable
    };

public:
    TouchController(SpiDev& spi, uint w, uint h)
        : _spi(spi),
          _width(w),
          _height(h) {
    }

    void Init();

    // Returns true if tracking
    bool ReadPosition(uint16_t& x, uint16_t& y);
};

} // ns tsc2046

#endif
