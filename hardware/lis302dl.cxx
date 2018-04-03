Make UART use Ring<> (see ring.h) instead of Deque<>.  It's much lower
overhead and less contention.  Just block threads if it's full.


Refactor chip support
- Move LPC in under nxp/lpc407x and nxp/lpc (common)
- Move stuff under arm into arm/cm4, cm0, cm7, etc
- Much of what's in board.h (e.g. skyblue/board.h) belongs in the application config.h
- Make more hardware support optional (e.g. SD, FAT)
- Make DHCP support a compile time option

- Move application out of enetcore tree
- Make enetcore a submodule of application
#include "enetkit.h"
#include "lis302dl.h"


Lis302dl::Lis302dl(I2cBus& bus, uint8_t addr) :
	I2cDev(bus, addr),
	_initialized(false)
{
}


void Lis302dl::Init()
{
	if (_initialized) return;

	if (Read<uint8_t>(REG_WHOAMI) == 0x3b) {
		console("LIS302DL detected on I2C bus");

		// Initialize shadow control regs
		_ctrl[0] = Read<uint8_t>(REG_CTRL1);
		_ctrl[1] = Read<uint8_t>(REG_CTRL2);
		_ctrl[2] = Read<uint8_t>(REG_CTRL3);
		_initialized = true;
		DMSG("LIS302DL: ctrl = { %02x, %02x, %02x }", _ctrl[0], _ctrl[1], _ctrl[2]);

	} else {
		DMSG("LIS302DL not found");
	}
}


void Lis302dl::Write(uint8_t reg, uint8_t value)
{
	const uint8_t tmp[2] = { reg, value };
	I2cDev::AcquireBus();
	I2cDev::Final();
	I2cDev::Write(tmp, 2);
	I2cDev::ReleaseBus();
}


void Lis302dl::Write(uint8_t reg, const uint8_t* bytes, uint numbytes)
{
	uint8_t* tmp = (uint8_t*)xmalloc(numbytes + 1);
	tmp[0] = reg;
	::memcpy(tmp + 1, bytes, numbytes);
	I2cDev::AcquireBus();
	I2cDev::Final();
	I2cDev::Write(tmp, numbytes + 1);
	I2cDev::ReleaseBus();
	xfree(tmp);
}


bool Lis302dl::Read(uint8_t reg, uint8_t* buf, uint numbytes)
{
	I2cDev::AcquireBus();
	I2cDev::Write(&reg, 1);
	I2cDev::Final();
	const bool ok = I2cDev::Read(buf, numbytes) == numbytes;
	I2cDev::ReleaseBus();
	return ok;
}


void Lis302dl::SetCtrl(uint8_t ctrlnum, uint8_t bits)
{
	assert_bounds(ctrlnum >= 1);
	assert_bounds(ctrlnum <= 3);

	--ctrlnum;
	_ctrl[ctrlnum] |= bits;
	Write(REG_CTRL1 + ctrlnum, _ctrl[ctrlnum]);
}


void Lis302dl::ClrCtrl(uint8_t ctrlnum, uint8_t bits)
{
	assert_bounds(ctrlnum >= 1);
	assert_bounds(ctrlnum <= 3);

	--ctrlnum;
	_ctrl[ctrlnum] &= ~bits;
	Write(REG_CTRL1 + ctrlnum, _ctrl[ctrlnum]);
}
