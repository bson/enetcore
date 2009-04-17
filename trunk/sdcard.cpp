#include "enetkit.h"
#include "sdcard.h"
#include "spi.h"
#include "crc32.h"


SDCard _sd(_spi0);

SDCard::SDCard(SPI& spi) : _spi(spi) { }

void SDCard::Init()
{
	Mutex::Scoped L(_lock);

	if (_initialized) return;

	_spi.Init();
	_spi.SetSpeed(100000);

	_spi.Select();
	
	DMSG("SDCard: reset");

	uint8_t status = 0xff;

	for (uint i = 0; i < 100 && status != 1; ++i)
		status = SendCMD(0);

	if (status != 1) {
		abort();
		DMSG("SDCard: CMD0 failed - missing card?");
		_spi.Deselect();
		return;
	}
	
	const uint8_t value = SendCMD(8, 0, 1, 0xaa);
	if (value == 0xff) {
		DMSG("SDCard: initialization failed");
		_spi.Deselect();
		return;
	}

	_version2 = value != 5;		// 5: Invalid command - not a 2.00 card

	console("Version %u SD Card", (int)_version2 + 1);

	for (uint i = 0; i < 100; ++i) {
		if (!SendACMD(41)) {
			_initialized = true;
			break;
		}
	}
	
	if (!_initialized) {
		console("SDCard: initialization failed - card not ready");
	} else {
		_spi.SetSpeed(24000000);
	}
}


uint8_t SDCard::SendCMD(uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	_lock.AssertLocked();

	const uint8_t cmdbuf[] = { 0xff, 0x40 | cmd, a >> 8, a, b, c, cmd ? 0 : 0x95, 0xff };

	_spi.Send(cmdbuf, sizeof cmdbuf);
	return _spi.Read();
}


uint8_t SDCard::SendACMD(uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	uint8_t result = SendCMD(55);
	if (result != 1) {
		DMSG("SDCard: CMD55 failed");
		return 0xff;
	}
	return SendCMD(cmd, a, b, c);
}


bool SDCard::ReadSector(uint secnum, void* buf)
{
	Mutex::Scoped L(_lock);

	const uint pos = 512 * secnum;

	bool ok;
	uint tries = 4;				// Retry a few times on CRC error
	do {
		SendCMD(17, pos >> 16, pos >> 8, pos);
		const uint8_t result = _spi.Read();
		if (result == 0xff) return false;

		// Wait up to 10 msec for a reply, retrying in 250usec intervals (40 times 100 usec)
		const uint8_t b1 = _spi.ReadReply(250, 40);
		if (result || b1 != 0xfe) return false;

		_spi.ReadBuffer(buf, 512);

		const uint16_t crc_sent = (_spi.Read() << 8) | _spi.Read();
		const uint32_t crc32 = Crc32::Checksum(buf, 512);
		ok = (uint16_t)crc32 == crc_sent;
	}
	while (!ok && --tries);

	return ok;
}
