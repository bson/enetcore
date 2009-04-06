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

	_spi.SetReadBuf(520);
	_spi.SetWriteBuf(520);
	_spi.Select();
	
	DMSG("SDCard: performing CMD0");

	uint8_t value = SendCMD(0);
	if (value != 1) {
		DMSG("SDCard: CMD0 failed - no card in slot?");
		_spi.Deselect();
		return;
	}
	
	value = SendCMD(8, 0, 1, 0xaa);
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
		value = SendACMD(41);
		if (value == 0) {
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

	return SendBytes(cmdbuf, sizeof cmdbuf);
}


uint8_t SDCard::SendBytes(const void* buf, uint numbytes, uint readcount)
{
	const Time deadline = Time::Now() + Time::FromMsec(20);

	_spi.SetReadCount(readcount);
	do {
		_spi.ReadClear();
		_spi.Send(buf, numbytes);
		if (_spi.WaitRead(Time::Now() + Time::FromUsec(10000)))
			return _spi[0];
	} while (Time::Now() < deadline);

	return 0xff;
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

	DMSG("Sending CMD17 to read");

	uint8_t result = SendCMD(17, pos >> 16, pos >> 8, pos);
	if (result == 0xff) return false;

	DMSG("Getting first block");

	const static char ff = 0xff;
	uint8_t b1 = SendBytes(&ff, 1);

	DMSG("Result = %u, block1 = %x", result, b1);

	if (result != 0 || b1 != 0xfe) return false;

	result = SendBytes(&ff, 1, 512);
	
	DMSG("Successfully read sector");

	_spi.Recv(buf, 512);
	_spi.ReadClear();

	for (;;) ;
}
