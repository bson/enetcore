// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "sdcard.h"
#include "board.h"


SDCard::SDCard(SpiDev& spi) :
	_spi(spi)
{
	_drivelock = NULL;
	_inuse = 0;
	_initialized = false;
	_version2 = false;
	_sdhc = false;
}


void SDCard::SetLock(Output *lock)
{
	_drivelock = lock;
}


bool SDCard::Init()
{
	Mutex::Scoped L(_lock);

	if (_initialized)
        return true;

	_spi.Init();
	_spi.SetSpeed(100000);

	_spi.Select();

	int value = -1;

	for (uint i = 0; i < 100 && value != 1; ++i)
		value = SendCMD(0);

	if (value != 1) {
		DMSG("SDCard: CMD0 failed - missing card?");
		_spi.Deselect();
		return false;
	}
	
	value = SendCMD(8, 0, 1, 0xaa);
	if (value == -1) {
		DMSG("SDCard: initialization failed");
		_spi.Deselect();
		return false;
	}

	Retain();

	_sdhc = false;
	_version2 = value != 5;		// 5: Invalid command - not a 2.00 card

	// The SD Card Simplified Physical Layer Spec V2.00 includes a CMD58 (read OCR)
	// during initialization.  While optional prior to ACMD41, we transmit it just
	// in case some cards require it to advance state during init.  We don't really
	// care about the result - it mainly specifies operating voltages, and we have
	// no control over those.
	const uint64_t r3 = SendCMDR(5, 58);
	if ((r3 >> 32) != 1)
        return false; // Card should be initializing at this point

	const uint32_t ocr = (uint32_t)r3;

	DMSG("SD card OCR=0x%x", ocr);

	const Time deadline = Time::Now() + Time::FromMsec(1500); // 1.5sec limit is somewhat arbitrary
	while (!_initialized && Time::Now() < deadline) {
		const int r1 = SendACMD(41, 0x4000); // 0x40000000 = host supports high capacity
		const int r2 = _spi.Read(); // Some devices aren't quick enough to respond immediately
		const uint8_t response = (r1 != -1 ? r1 : r2);

		if (response != 0xff && (response & ~1)) {
			DMSG("SD Card: command error %x", response);
		} else if (!response) {
			_initialized = true;
		} else {
            Thread::Delay(25);
		}
	}

	if (_version2) {
		const uint64_t r3 = SendCMDR(5, 58);
		if ((r3 >> 32) != 0) 
			_initialized = false; // R1 part of R3: should no longer be initializing

		const uint32_t ocr = (uint32_t)r3;
		if (ocr & 0x80000000)
            _sdhc = (ocr & 0x40000000) != 0;
	}


	if (!_sdhc) {
		value = SendCMD(16, 0, 2, 0); // SET_BLOCKLEN(512)
	}

	if (_initialized) {
		_spi.SetSpeed(24000000);
		console("Version %u SD%s Card in slot", _version2 + 1, _sdhc ? "HC" : "");
	} else {
		console("SDCard: initialization failed - card not ready");
	}

	Release();

	return _initialized;
}


int SDCard::SendCMD(uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	_lock.AssertLocked();

	const uint8_t cmdbuf[] = { 0xff, uint8_t(0x40 | cmd), uint8_t(a >> 8),
                               uint8_t(a), b, c, uint8_t(cmd ? 0 : 0x95), 0xff };

	_spi.Send(cmdbuf, sizeof cmdbuf);
    return _spi.Read();
}


uint64_t SDCard::SendCMDR(uint8_t num, uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	assert(num);
    const int r = SendCMD(cmd, a, b, c);
    uint64_t result = (r & 0xff);

	while (--num) {
        const int tmp = _spi.Read();
        result = (result << 8) | (tmp != -1 ? tmp : 0xff);
    }
    
	return result;
}


int SDCard::SendACMD(uint8_t cmd, uint16_t a, uint8_t b, uint8_t c)
{
	uint8_t result = SendCMD(55);
	if (result != 1) {
		DMSG("SDCard: CMD55 failed");
		return -1;
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
		const Time rdstart = Time::Now();

		int result = SendCMD(17, pos >> 16, pos >> 8, pos);
		for (uint i = 0; i < 10; ++i) {
			if (result != -1)
                break;
            
			result = _spi.Read();
		}

		if (result == -1) {
			DMSG("SD: no response to read command");
			return false;
		}

		// Wait up to 100 msec for a reply, retrying in 250usec
		// intervals (400 times 250 usec)
		const int b1 = _spi.ReadReply(250, 400);
		if (result || b1 != 0xfe) {
			DMSG("SD: Read Command failed: %x, %x", result, b1);
			return false;
		}
		DMSG("SD read: %u usec", (uint)(Time::Now() - rdstart).GetUsec());

		const Time txstart = Time::Now();
		CrcCCITT crc16(0);
		_spi.ReadBuffer(buf, 512, &crc16);

		DMSG("SD sector transfer: %u usec", (uint)(Time::Now() - txstart).GetUsec());

		const uint16_t crc_sent = (_spi.Read() << 8) | _spi.Read();
		crcok = (uint16_t)crc16.GetValue() == crc_sent;
		if (!crcok)
            DMSG("SD: CRC error, sector 0x%x, expected 0x%x, got 0x%x",
                 secnum, crc_sent, crc16.GetValue());
	}
	while (!crcok && --tries);

	return crcok;
}


void SDCard::Retain()
{
	Mutex::Scoped L(_lock);
	if (!_inuse++ && _drivelock)
        _drivelock->Raise();
}


void SDCard::Release()
{
	Mutex::Scoped L(_lock);
	assert(_inuse);
	if (!--_inuse && _drivelock)
        _drivelock->Lower();
}
