#include "enetkit.h"
#include "thread.h"


LpcSpiBus::LpcSpiBus(uint32_t base)
{
	_base = (volatile uint8_t*)base;
	_prescaler = 254;
}


void LpcSpiBus::Init()
{
}


void LpcSpiBus::SetSpeed(uint hz)
{
	const uint scaler = PCLK/hz & ~1;

	_prescaler = max(min(scaler, (uint)254), (uint)8);
	_base[SPI_SPCCR] = _prescaler;

	DMSG("SpiBus: Prescaler = %u, clock = %u kHz", _prescaler, PCLK/_prescaler/1000);

	// SPIE=0, LSBF=0, MSTR=1, CPOL=1, CPHA=0
	_base[SPI_SPCR] = 0b00110000;

	for (uint i = 0; i < 20; ++i) {
		_base[SPI_SPDR] = 0xff;
		while (!(_base[SPI_SPSR] & 0x80)) ;
	}

	_base[SPI_SPCR] = 0b10110000;
}

uint8_t LpcSpiBus::Read(uint8_t code)
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


uint8_t LpcSpiBus::Send(const uint8_t* s, uint len)
{
	uint8_t tmp = 0xff;
	while (len--)
		tmp = Read(*s++);

	return tmp;
}


uint8_t LpcSpiBus::ReadReply(uint interval, uint num_tries, uint8_t code)
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


bool LpcSpiBus::ReadBuffer(void* buffer, uint len, Crc16* crc)
{
	if (!len) return true;

	uint8_t* p = (uint8_t*)buffer;
	bool ok = true;

	_base[SPI_SPDR] = 0xff;

	while (len--) {
		uint spsr;
		while (!((spsr = _base[SPI_SPSR]) & 0x80)) continue;

		ok &= (spsr & 0xf8) == 0x80;

		// Read received byte and start transmission of next if anything left
		const uint tmp = _base[SPI_SPDR];
		if (len) _base[SPI_SPDR] = 0xff;

		// Then during SPI clocking store the value read and update
		// running Crc
		*p++ = tmp;
		crc->Update(tmp);
	}
	return ok;
}


LpcSpiDev::LpcSpiDev(LpcSpiBus& bus) :
	_bus(bus),
	_ssel(NULL),
	_speed(100000),
	_selected(false)
{
}

void LpcSpiDev::Select()
{
	if (!_selected)  {
		_bus.SetSpeed(_speed);
		if (_ssel) _ssel->Raise();
		_selected = true;
	}
}


void LpcSpiDev::Deselect()
{
	if (_selected) {
		if (_ssel) _ssel->Lower();
		_selected = false;
	}
}


void LpcSpiDev::SetSpeed(uint hz) 
{
	_speed = hz;
	if (_selected) _bus.SetSpeed(_speed);
}
