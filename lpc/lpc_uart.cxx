// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "thread.h"
#include "lpc_uart.h"


void LpcUart::Init(uint speed, uint framing)
{
    // Speed, somewhat low
	const uint prescale = (PCLK / speed + 8) / 16;

    // Actual speed
    const uint actual = PCLK / prescale / 16;

    ScopedNoInt G;

	_base[REG_LCR] = 0x80 | framing; // Set framing and access divisor
	_base[REG_DLL] = prescale & 0xff;
	_base[REG_DLM] = prescale / 256;
	_base[REG_LCR] = framing;	// Disable divisor access

	_base[REG_FCR] = FCR_FIFOEN | FCR_RXFIFORES | FCR_TXFIFORES | FCR_RXTRIGLVL_14;

    _base[REG_FDR] = 0x10;      // Don't use FDR
    _base[REG_TER] = BIT7;      // Enable transmitter

    _recvq.Reserve(256);
    _recvq.SetAutoResize(false);

    _sendq.Reserve(256);
    _sendq.SetAutoResize(true);
}


void LpcUart::Write(const String& s)
{
    ScopedNoInt G;

    _sendq.PushBack(s.CStr(), s.Size());

    FillTxFifo();

    _sendq.AutoCompact();
}


void LpcUart::WriteCStr(const char* s) {
    ScopedNoInt G;

    _sendq.PushBack((const uint8_t*)s);

    FillTxFifo();

	_sendq.AutoCompact();
}
    

// Call wit interrupts disabled or interrupt handler
void LpcUart::FillTxFifo()
{
    assert(!IntEnabled());

	_sendq.SetAutoResize(false);

	while (!_sendq.Empty() && (_base[REG_LSR] & LSR_THRE)) {
		_base[REG_THR] = _sendq.Front();
		_sendq.PopFront();
	}
	_sendq.SetAutoResize(true);
}


void LpcUart::SyncDrain()
{
    ScopedNoInt G;

	while (!_sendq.Empty())
        FillTxFifo();
}


void LpcUart::HandleInterrupt()
{
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
        ScopedNoInt G;

        // Character timeout indicator - drain FIFO
		while (_base[REG_LSR] & LSR_RDR) {
			const uchar c = _base[REG_RBR];
            if (_recvq.Headroom() > 0)
                _recvq.PushBack(c);
		}

		// Fallthru - fill up Tx FIFO while we're here
	}
	case 0b0010: {
		// THRE
        ScopedNoInt G;

		FillTxFifo();
	}
	}
}


void LpcUart::SetInterrupts(bool enable)
{
	_base[REG_IER] = enable ? 0b11 : 0;		// RBR, THRE
}


void LpcUart::Interrupt(void *token)
{
    ((LpcUart*)token)->HandleInterrupt();
}
