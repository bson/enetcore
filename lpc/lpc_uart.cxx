// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "thread.h"
#include "ring.h"
#include "lpc_uart.h"


void LpcUart::Init(uint speed, uint framing)
{
    // Speed, somewhat low
	const uint prescale = (PCLK / speed + 8) / 16;

    // Actual speed
    const uint actual = PCLK / prescale / 16;

    Thread::IPL G(IPL_UART-1);

	_base[REG_LCR] = 0x80 | framing; // Set framing and access divisor
	_base[REG_DLL] = prescale & 0xff;
	_base[REG_DLM] = prescale / 256;
	_base[REG_LCR] = framing;	// Disable divisor access

	_base[REG_FCR] = FCR_FIFOEN | FCR_RXFIFORES | FCR_TXFIFORES | FCR_RXTRIGLVL_14;

    _base[REG_FDR] = 0x10;      // Don't use FDR
    _base[REG_TER] = BIT7;      // Enable transmitter
}


void LpcUart::Write(const uint8_t *data, uint len)
{
    Mutex::Scoped L(_w_mutex);

    Thread::IPL G(IPL_UART-1);

    for (const uint8_t *p = data; p < data + len; ) {
        if (_sendq.Headroom()) {
            while (_sendq.Headroom() && (p < data + len))
                _sendq.PushBack(*p++);

            FillTxFifo();
        }
        if (p < data + len)
            Thread::WaitFor(this);
    }
}


void LpcUart::Write(const String& s)
{
    Write(s.CStr(), s.Size());
}


void LpcUart::WriteCStr(const char* s) {
    Write((const uint8_t*)s, strlen(s));
}
    

// Call with interrupts disabled or interrupt handler
void LpcUart::FillTxFifo()
{
	while (!_sendq.Empty() && (_base[REG_LSR] & LSR_THRE)) {
		_base[REG_THR] = _sendq.PopFront();
	}
}

void LpcUart::SyncDrain()
{
    ScopedNoInt G;

	while (!_sendq.Empty())
        FillTxFifo();
}

void LpcUart::HandleInterrupt()
{
    bool wake = false;
	const uint32_t iir = _base[REG_IIR];

	// Switch in order of priority - highest at top, then fall through and
	// check if we should service lower priority conditions while here.
	switch (iir & 0b1111) {
	case 0b0110: {
		// Receiver line status
		const uint tmp = _base[REG_LSR];		// Clear by reading LSR
		// Fallthru - drain RBR while at it
	}
	case 0b0100:
		// Receiver - FIFO triggered - drain
	case 0b1100: {
        // Character timeout indicator - drain FIFO
		while (_base[REG_LSR] & LSR_RDR) {
			const uchar c = _base[REG_RBR];
            if (_recvq.Headroom() > 0) {
                _recvq.PushBack(c);
                wake = true;
            }
		}

		// Fallthru - fill up Tx FIFO while we're here
	}
	case 0b0010: {
        const bool above = _sendq.Size() >= BUFSIZE / 4;

		// THRE
		FillTxFifo();

        // If we were above a 1/4 low water mark and went below it, wake
        if (above && _sendq.Size() < BUFSIZE / 4)
            wake = true;
        // Or, if we just drained the buffer
        else if (_sendq.Empty())
            wake = true;
            
	}
	}

    if (wake)
        Thread::WakeAll(this);
}


void LpcUart::SetInterrupts(bool enable)
{
	_base[REG_IER] = enable ? 0b11 : 0;		// RBR, THRE
}


void LpcUart::Interrupt(void *token)
{
    ((LpcUart*)token)->HandleInterrupt();
}
