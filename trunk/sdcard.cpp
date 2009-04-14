#include "enetkit.h"
#include "sdcard.h"
#include "spi.h"


SDCard _sd(_spi0);

SDCard::SDCard(SPI& spi) : _spi(spi) { }

void SDCard::Init()
{
	Mutex::Scoped L(_lock);

	if (_initialized) return;

	_spi.Init();
	_spi0.SetSpeed(600000);

	_spi.Select();
	
	DMSG("SDCard: performing CMD0");

	if (SendCMD(0) != 1) {
		DMSG("SDCard: CMD0 failed - no card in slot?");
		_spi.Deselect();
		return;
	}
	
	const uint8_t value = SendCMD(8, 0, 1, 0xaa);
	if (value == 0xff) {
		DMSG("SDCard: initialization failed");
		_spi.Deselect();
		return;
	}

	_version2 = true;
	if (value == 5) {
		// Invalid command - not a 2.00 card
		DMSG("Version 1 SD Card");
		_version2 = false;
	} else {
		DMSG("Version 2 SD Card");
	}

	for (uint i = 0; i < 100; ++i) {
		if (!SendACMD(41)) {
			_initialized = true;
			break;
		}
	}
	
	if (!_initialized) {
		console("SDCard: initialization failed - card not ready");
	}
}


uint8_t SDCard::SendCMD(uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	_lock.AssertLocked();

	const uint8_t cmdbuf[] = { 0xff, 0x40 | cmd, a >> 8, a, b, c, cmd ? 0 : 0x95, 0xff };

	return _spi.Send(cmdbuf, sizeof cmdbuf);
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


bool SDCard::ReadSector(uint secnum, Deque<uint8_t>& buf)
{
	Mutex::Scoped L(_lock);

	const uint pos = 512 * secnum;

	DMSG("Sending CMD17 to read");

	const uint8_t result = SendCMD(17, pos >> 16, pos >> 8, pos);
	if (result == 0xff) return false;

	DMSG("Getting first block");

	Time t = Time::Now();

	// Wait up to 10 msec for a reply, polling continuously
	const uint8_t b1 = _spi.ReadReply(0, 10000);

	DMSG("Result = %u, block1 = %x", result, b1);

	if (result || b1 != 0xfe) return false;
	
	_spi.ReadBuffer(buf, 514);

	const uint16_t crc = (_spi.Read() << 8) | _spi.Read();
	
	t = Time::Now() - t;

	DMSG("Successfully read sector in %u usec", t.GetUsec());
}
