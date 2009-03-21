#include "enetkit.h"
#include "serial.h"
#include "thread.h"


SerialPort _uart0((volatile void*)UART0_BASE, 115200);
SerialPort _uart1((volatile void*)UART1_BASE, 9600);


void SerialPort::SetSpeed(uint speed, uint framing)
{
	const uint prescale = PCLK / speed / 16;

	Spinlock::Scoped L(_lock);

	_base[UART_LCR] = 0x80 | framing; // Set framing and access divisor
	_base[UART_DLL] = prescale & 0xff;
	_base[UART_DLM] = prescale / 256;
	_base[UART_LCR] = framing;	// Disable divisor access
	_base[UART_FCR] = 0b11000111;	 // Reset, set FIFO Rx trigger to 14 bytes
}


void SerialPort::WriteSync(const uchar* buf, uint len)
{
	while (len--) {
		// Spin until THRE
		for (;;) {
			Spinlock::Scoped L(_lock);

			if (_base[UART_LSR] & 0b100000) {
				_base[UART_THR] = *buf++;
				break;
			}
		}
	}
}


void SerialPort::WriteSync(const String& s)
{
	WriteSync(s.CStr(), s.Size());
}


void SerialPort::Write(const String& s)
{
	Spinlock::Scoped L(_lock);

	_sendq.PushBack(s.CStr(), s.Size());

	FillFifo();
}


void SerialPort::FillFifo()
{
	Spinlock::Scoped L(_lock);

	while (!_sendq.Empty() && (_base[UART_LSR] & 0b100000)) {
		uchar c = _sendq.Front();
		_sendq.PopFront();
		_base[UART_THR] = c;
	}
}


// * static __irq
void SerialPort::Interrupt()
{
	SaveStateExc(4);

	if (_vic.ChannelPending(6))
		_uart0.HandleInterrupt();

	if (_vic.ChannelPending(7))
		_uart1.HandleInterrupt();

	_vic.ClearPending();

	LoadStateReturnExc();
}


void SerialPort::HandleInterrupt()
{
	Spinlock::Scoped L(_lock);
	const uint32_t iir = _base[UART_IIR];

	// Switch in order of priority - highest at top, then fall through and
	// check if we should service lower priority conditions while here.
	switch (iir & 0b1111) {
	case 0b0110: {
		// Receiver line status
		uint tmp = _base[UART_LSR];		// Clear by reading LSR
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


void SerialPort::SetInterrupts(bool enable)
{
	assert(enable);

	Spinlock::Scoped L(_lock);
	_base[UART_IER] = 0b11;		// RBR, THRE
}
