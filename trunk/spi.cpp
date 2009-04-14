#include "enetkit.h"
#include "spi.h"
#include "thread.h"


SPI _spi0(SPI0_BASE);


SPI::SPI(uint32_t base)
{
	_base = (volatile uint8_t*)base;
}


void SPI::Init()
{
	Deselect();
}


void SPI::SetSpeed(uint hz)
{
	Select();

	const uint scaler = PCLK/hz & ~1;
	DMSG("SPI: Prescaler = %u", max(min(scaler, (uint)254), (uint)8));
	_base[SPCCR] = max(min(scaler, (uint)254), (uint)8);

	// SPIE=1, LSBF=0, MSTR=1, CPOL=1, CPHA=0
	_base[SPCR] = 0b00110000;

	for (uint i = 0; i < 20; ++i) {
		_base[SPDR] = 0xff;
		while (!(_base[SPSR] & 0x80)) ;
	}

	_base[SPCR] = 0b10110000;
}


void SPI::Select()
{
	IO0CLR = 1 << 10;
}


void SPI::Deselect()
{
	IO0SET = 1 << 10;
}


uint8_t SPI::Read(uint8_t code)
{
	while (!(_base[SPSR] & 0x80)) continue;

	if ((_base[SPSR] & 0b11111000) == 0x80) {
		_base[SPDR] = code;
		return _base[SPDR];
	} else {
		const uint tmp = _base[SPDR];
		return 0xff;
	}
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

		if (interval)  udelay(interval);
	}

	return 0xff;
}


void SPI::ReadBuffer(Deque<uint8_t>& buffer, uint len)
{
	while (len--)
		buffer.PushBack(Read());
}


bool SPI::WaitReady(uint interval, uint num_tries, uint8_t value)
{
	while (num_tries--) {
		const uint8_t tmp = Read();
		if (tmp == value)  return true;

		udelay(interval);
	}
}
