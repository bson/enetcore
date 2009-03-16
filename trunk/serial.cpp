#include "enetcore.h"
#include "serial.h"


SerialPort _console((volatile void*)UART0_BASE);
SerialPort _lcd((volatile void*)UART1_BASE);


void SerialPort::SetSpeed(uint speed)
{
	const uint prescale = PCLK / speed / 16;

	_base[UART_LCR] = 0x83;		// Set 8N1 and access divisor
	_base[UART_DLL] = prescale & 0xff;
	_base[UART_DLM] = prescale / 256;
	_base[UART_LCR] = 3;		// Disable divisor access
	_base[UART_FCR] = 7;	 // Reset, set FIFO Rx trigger to 14 bytes
}


void SerialPort::WriteSync(const uchar* buf, uint len)
{
	while (len--) {
		while (!(_base[UART_LSR] & 16)) continue; // Spin until THRE
		_base[UART_THR] = *buf++;
	}
}
