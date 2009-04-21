#include "enetkit.h"
#include "spi.h"
#include "thread.h"


SPI _spi0(SPI0_BASE);
SPI _spi1(SPI1_BASE);


SPI::SPI(uint32_t base)
{
	_base = (volatile uint8_t*)base;
	_ssel = NULL;
}


void SPI::Init(Output* ssel)
{
	Deselect();
	_ssel = ssel;
}


void SPI::SetSpeed(uint hz)
{
	Select();

	const uint scaler = PCLK/hz & ~1;
	const uint prescaler = max(min(scaler, (uint)254), (uint)8);
	_base[SPI_SPCCR] = prescaler;

	DMSG("SPI: Prescaler = %u, clock = %u kHz", prescaler, PCLK/prescaler/1000);

	// SPIE=1, LSBF=0, MSTR=1, CPOL=1, CPHA=0
	_base[SPI_SPCR] = 0b00110000;

	for (uint i = 0; i < 20; ++i) {
		_base[SPI_SPDR] = 0xff;
		while (!(_base[SPI_SPSR] & 0x80)) ;
	}

	_base[SPI_SPCR] = 0b10110000;
}


void SPI::Select()
{
	if (_ssel) _ssel->Raise();
}


void SPI::Deselect()
{
	if (_ssel) _ssel->Lower();
}


uint8_t SPI::Read(uint8_t code)
{
	// Xmit
	_base[SPI_SPDR] = code;

	// Wait for clocking to finish
	while (!(_base[SPI_SPSR] & 0x80)) continue;

	const bool ok = (_base[SPI_SPSR] & 0b11111000) == 0x80;

	// Always read, to clear SPSR SPIF (0x80) flag
	const uint8_t tmp =  _base[SPI_SPDR];

	return ok ? tmp : 0xff;
}


uint8_t SPI::Send(const uint8_t* s, uint len)
{
	uint8_t tmp = 0xff;
	while (len--)
		tmp = Read(*s++);

	return tmp;
}


uint8_t SPI::ReadReply(uint interval, uint num_tries, uint8_t code)
{
	while (num_tries--) {
		const uint8_t tmp = Read(code);
		if (tmp != 0xff) return tmp;

		if (interval)  {
			// If delay is > 25 usec, use scheduler
			if (interval > 25)
				Self().Sleep(Time::FromUsec(interval));
			else
				udelay(interval);
		}
	}

	return 0xff;
}


void SPI::ReadBuffer(void* buffer, uint len)
{
	uint8_t* p = (uint8_t*)buffer;

	while (len--) *p++ = Read();
}


bool SPI::WaitReady(uint interval, uint num_tries, uint8_t value)
{
	while (num_tries--) {
		const uint8_t tmp = Read();
		if (tmp == value)  return true;

		udelay(interval);
	}
}
