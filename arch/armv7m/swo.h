// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef _SWO_H_
#define _SWO_H_

#include "core/bits.h"
#include "core/mutex.h"
#include "core/consumer.h"
#include "core/pstring.h"


class Swo: public Consumer<uint8_t> {
    enum {
        PORT = 0,               // Use port 0
        TRACE_ID = 1,           // TPIU source ID

        // ITM_TCR - trace control
        BUSY = 23,
        SOURCE = 16,
        TSPSC = 8,
        SWOENA = 4,
        DWTENA = 3,
        SYNCENA = 2,
        TSENA = 1,
        ITMENA = 0,

        // TPIU_CSPSR values bit 1:0
        TPIU_SYNC_MODE = 0b00,
        TPIU_SWO_MANCH = 0b01,  // Manchester, default
        TPIU_SWO_NRZ   = 0b10,

        // DBG_CR
        TRACE_MODE = 6,
        TRACE_IOEN = 5,

        // DBG_DEMCR
        TRCENA = 24,
    };

    mutable Mutex _lock;

public:
    // Global one-time enable and configuration
    void Enable(uint32_t bps) {
        const uint32_t prescaler = CCLK/bps - 1;
        assert(prescaler <= 0xffff);
        assert(TPIU_DEVID & BIT(11)); // UART/NRZ supported

        // Enable TRACESWO pin, mode 0 = prescaled async/uart
        DBG_CR = (DBG_CR & ~((0b11 << TRACE_MODE) | BIT(TRACE_IOEN))) | BIT(TRACE_IOEN);

        // Enable all SWT/ITM/TPIU features
        DBG_DEMCR |= BIT(TRCENA);

        // Configure test stimulus port 0
        TPIU_CSPSR = (TPIU_CSPSR & ~0b1111) | 0b0001; // 1-bit port
        TPIU_SPPR = (TPIU_SPPR & ~0b11) | TPIU_SWO_NRZ;
        TPIU_ACPR = (TPIU_ACPR & ~0xffff) | prescaler;

        // Enable trace module
        ITM_LAR = ITM_UNLOCK_KEY;
        ITM_TCR &= ~BIT(ITMENA);
        while (ITM_TCR & BIT(BUSY))
            ;

        ITM_TCR = BIT(SYNCENA) | BIT(SWOENA) | (TRACE_ID << SOURCE);
        ITM_TCR |= BIT(ITMENA);
        ITM_TPR = 0;
        ITM_TER |= BIT(PORT);
        ITM_LAR = 0;
    }

    void Write(const uint8_t* buf, uint len) {
        assert(ITM_TCR & BIT(ITMENA));

        Mutex::Scoped L(_lock);

        while (len--) {
            while (ITM_STIM0 & 0xff)
                ;

            (volatile uint8_t&)ITM_STIM0 = *buf++;
        }
    }

    // Consumer interface
    void Apply(const uint8_t* buf, uint len) { Write(buf, len); }
    void Apply(uint8_t c) { Write(&c, 1); }
    void Apply(const class String& s) { Write(s.CStr(), s.Size()); }
};

#endif  // _SWO_H_
