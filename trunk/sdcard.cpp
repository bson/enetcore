#include "enetkit.h"
#include "sdcard.h"
#include "spi.h"
#include "crc16.h"


SDCard _sd(_spi0);

SDCard::SDCard(SPI& spi) : _spi(spi) { }

bool SDCard::Init()
{
	Mutex::Scoped L(_lock);

	if (_initialized) return true;

	_spi.Init();
	_spi.SetSpeed(100000);

	_spi.Select();
	
	DMSG("SDCard: init");

	uint8_t status = 0xff;

	for (uint i = 0; i < 100 && status != 1; ++i)
		status = SendCMD(0);

	if (status != 1) {
		abort();
		DMSG("SDCard: CMD0 failed - missing card?");
		_spi.Deselect();
		return false;
	}
	
	const uint8_t value = SendCMD(8, 0, 1, 0xaa);
	if (value == 0xff) {
		DMSG("SDCard: initialization failed");
		_spi.Deselect();
		return false;
	}

	_sdhc = false;
	_version2 = value != 5;		// 5: Invalid command - not a 2.00 card

	const uint64_t r3 = SendCMDR(5, 58);
	if ((r3 >> 32) != 1)  return false; // Card should be initializing at this point

	const uint32_t ocr = (uint32_t)r3;
	DMSG("SD card OCR=0x%x", ocr);

	const Time deadline = Time::Now() + Time::FromMsec(250);
	while (!_initialized && Time::Now() < deadline) {
		const uint8_t r1 = SendACMD(41, 0x4000); // 0x40000000 = host supports high capacity
		const uint8_t r2 = _spi.Read(); // Some devices aren't quick enough to respond immediately
		const uint8_t response = (r1 != 0xff ? r1 : r2);

		if (response != 0xff && (response & ~1)) {
			DMSG("SD Card: command error %x", response);
		} else if (!response) {
			_initialized = true;
		} else {
			Self().Delay(25);
		}
	}

	if (_version2) {
		const uint64_t r3 = SendCMDR(5, 58);
		if ((r3 >> 32) != 0)  return false; // R1 part of R3: should no longer be initializing

		const uint32_t ocr = (uint32_t)r3;
		if (ocr & 0x80000000)  _sdhc = (ocr & 0x40000000) != 0;
	}


	if (_initialized) {
		_spi.SetSpeed(24000000);
		console("Version %u SD%s Card in slot", _version2 + 1, _sdhc ? "HC" : "");
	} else {
		console("SDCard: initialization failed - card not ready");
	}

	return _initialized;
}


uint8_t SDCard::SendCMD(uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	_lock.AssertLocked();

	const uint8_t cmdbuf[] = { 0xff, 0x40 | cmd, a >> 8, a, b, c, cmd ? 0 : 0x95, 0xff };

	_spi.Send(cmdbuf, sizeof cmdbuf);
	return _spi.Read();
}


uint64_t SDCard::SendCMDR(uint8_t num, uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	assert(num);
	uint64_t result = SendCMD(cmd, a, b, c);
	while (--num) result = (result << 8) | _spi.Read();
	return result;
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

	const uint pos = _sdhc ? secnum : 512 * secnum;

	bool crcok;
	uint tries = 4;				// Retry a few times on CRC error
	do {
		uint8_t result = SendCMD(17, pos >> 16, pos >> 8, pos);
		for (uint i = 0; i < 10; ++i) {
			if (result != 0xff) break;
			result = _spi.Read();
		}
		if (result == 0xff) {
			DMSG("SD: no response to read command");
			return false;
		}

		// Wait up to 100 msec for a reply, retrying in 250usec intervals (400 times 250 usec)
		const uint8_t b1 = _spi.ReadReply(250, 400);
		if (result || b1 != 0xfe) {
			DMSG("SD: Read Command failed: %x, %x", result, b1);
			return false;
		}

		_spi.ReadBuffer(buf, 512);

		const uint16_t crc_sent = (_spi.Read() << 8) | _spi.Read();
		const uint16_t crc16 = Crc16::Checksum(buf, 512);
		crcok = (uint16_t)crc16 == crc_sent;
		if (!crcok) DMSG("SD: CRC error, sector 0x%x", secnum);
	}
	while (!crcok && --tries);

	return crcok;
}
