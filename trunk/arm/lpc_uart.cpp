#include "enetkit.h"
#include "thread.h"


void LpcUart::SetSpeed(uint speed, uint framing)
{
	const uint prescale = PCLK / speed / 16;

	Spinlock::Scoped L(_lock);

	_base[UART_LCR] = 0x80 | framing; // Set framing and access divisor
	_base[UART_DLL] = prescale & 0xff;
	_base[UART_DLM] = prescale / 256;
	_base[UART_LCR] = framing;	// Disable divisor access
	_base[UART_FCR] = 0b11000111;	 // Reset, set FIFO Rx trigger to 14 bytes
}


void LpcUart::Write(const String& s)
{
	Spinlock::Scoped L(_lock);

	_sendq.PushBack(s.CStr(), s.Size());

	FillFifo();
	_sendq.AutoCompact();
}


void LpcUart::FillFifo()
{
	Spinlock::Scoped L(_lock);

	_sendq.SetAutoResize(false);

	while (!_sendq.Empty() && (_base[UART_LSR] & 0b100000)) {
		_base[UART_THR] = _sendq.Front();
		_sendq.PopFront();
	}
	_sendq.SetAutoResize(true);
}


void LpcUart::SyncDrain()
{
	Spinlock::Scoped L(_lock);

	_sendq.SetAutoResize(false);

	while (!_sendq.Empty()) {
		while (_base[UART_LSR] & 0b100000) {
			_base[UART_THR] = _sendq.Front();
			_sendq.PopFront();
		}
	}
	_sendq.SetAutoResize(true);
}


// * static __irq
void LpcUart::Interrupt()
{
	SaveStateExc(4);

	if (_vic.ChannelPending(INTCH_UART0))
		_uart0.HandleInterrupt();

	if (_vic.ChannelPending(INTCH_UART1))
		_uart1.HandleInterrupt();

	_vic.ClearPending();

	LoadStateReturnExc();
}


void LpcUart::HandleInterrupt()
{
	Spinlock::Scoped L(_lock);
	const uint32_t iir = _base[UART_IIR];

	// Switch in order of priority - highest at top, then fall through and
	// check if we should service lower priority conditions while here.
	switch (iir & 0b1111) {
	case 0b0110: {
		// Receiver line status
		const uint tmp = _base[UART_LSR];		// Clear by reading LSR
		// Fallthru - drain RBR while at it
	}
	case 0b0100:
		// Receiver - FIFO triggered - drain
	case 0b1100: {
		// Character timeout indicator - drain
		while (_base[UART_LSR] & 1) {
			const uchar c = _base[UART_RBR];
			_recvq.PushBack(c);
			// XXX echo to output
			_sendq.PushBack(c);
		}

		// Fallthru - fill up Tx FIFO
	}
	case 0b0010: {
		// THRE
		FillFifo();
	}
	}
}


void LpcUart::SetInterrupts(bool enable)
{
	assert(enable);

	Spinlock::Scoped L(_lock);
	_base[UART_IER] = 0b11;		// RBR, THRE
}
