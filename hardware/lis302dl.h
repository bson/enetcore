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
#ifndef __LIS302DL_H__
#define __LIS302DL_H__


// Device interface for LIS302DL accelerometer in I2C mode

class Lis302dl: public I2cDev {
	uint8_t _ctrl[3];			// Shadow CTRL reg 1-3
	bool _initialized;

public:
	// Note: Low bit of slave addr is hardware configurable through
	// the SDO pad.
	enum { SLAVE_ADDR = 0b0011100 };

	Lis302dl(I2cBus& bus = _i2c0, uint8_t addr = SLAVE_ADDR);

	// Device registers
	enum { REG_AUTO_INC = 0x80,
		   REG_WHOAMI = 0x0f,	// 0b00111011
		   REG_CTRL1 = 0x20,
		   REG_CTRL2 = 0x21,
		   REG_CTRL3 = 0x22,
		   REG_HP_FILTER_RESET = 0x23, // Read to reset high-pass filter
		   REG_STATUS = 0x27,
		   REG_OUTX = 0x29,
		   REG_OUTY = 0x2b,
		   REG_OUTZ = 0x2d,
		   REG_FF_WU_CFG_1 = 0x30,
		   REG_FF_WU_SRC_1 = 0x31, // ack1
		   REG_FF_WU_THS_1 = 0x32,
		   REG_FF_WU_DURATION_1 = 0x33,
		   REG_FF_WU_CFG_2 = 0x34,
		   REG_FF_WU_SRC_2 = 0x35, // ack1
		   REG_FF_WU_THS_2 = 0x36,
		   REG_FF_WU_DURATION_2 = 0x37,
		   REG_CLICK_CFG = 0x38,
		   REG_CLICK_SRC = 0x39,
		   REG_CLICK_THSY_X = 0x3b,
		   REG_CLICK_THSZ = 0x3c,
		   REG_CLICK_TimeLimit = 0x3d,
		   REG_CLICK_Latency = 0x3e,
		   REG_CLICK_Windows = 0x3f
	};

	// Common register bits
	enum { CTRL1_DR = 0x80,		// Data rate: 0 = 100Hz, 1 = 400Hz
		   CTRL1_PD = 0x40,		// Power down control
		   CTRL1_FS = 0x20,		// Full scale
		   CTRL1_STP = 0x10,	// Self test
		   CTRL1_STM = 0x08,	// Self test
		   CTRL1_Zen = 0x04,	// Z axis enable
		   CTRL1_Yen = 0x02,	// Y axis enable
		   CTRL1_Xen = 0x01,	// X axis enable

		   CTRL2_SIM = 0x80,	// SPI mode (0 = 4-wire, 1 = 3-wire)
		   CTRL2_BOOT = 0x40,	// Reboot (reset)
		   CTRL2_FDS = 0x10,	// Filtered data selection
		   CTRL2_HP_FF_WU2 = 0x08, // High pass filter enabled: free-fall/wake-up #2
		   CTRL2_HP_FF_WU1 = 0x04, // High pass filter enabled: free-fall/wake-up #1
		   CTRL2_HP_coeff2 = 0x02, // HP filter cutoff config
		   CTRL2_HP_coeff1 = 0x02, // HP filter cutoff config

		   CTRL3_IHL = 0x80,	// Interrupt: 1 = active low, 0 = active high
		   CTRL3_OD = 0x40,		// Interrupt open drain/push-pull
		   CTRL3_I2CFG2 = 0x20,	// Int 2 control
		   CTRL3_I2CFG1 = 0x10,	// Int 2 control
		   CTRL3_I2CFG0 = 0x08,	// Int 2 control
		   CTRL3_I1CFG2 = 0x04,	// Int 1 control
		   CTRL3_I1CFG1 = 0x02,	// Int 1 control
		   CTRL3_I1CFG0 = 0x01,	// Int 1 control

		   STAT_ZYXOR = 0x80,	// X|Y|Z overrun
		   STAT_ZOR = 0x40,		// Z overrun
		   STAT_YOR = 0x20,		// Y overrun
		   STAT_XOR = 0x10,		// X overrun
		   STAT_ZYXDA = 0x08,	// X|Y|Z data available
		   STAT_ZDA = 0x04,		// Z data available
		   STAT_YDA = 0x02,		// Y   - "" -
		   STAT_XDA = 0x01,		// X   - "" -
		   
	};

	void Init();

	void Write(uint8_t reg, uint8_t value);
	void Write(uint8_t reg, const uint8_t* bytes, uint numbytes);

	bool Read(uint8_t reg, uint8_t* buf, uint numbytes);

	// ctrlnum is 1-3 below
	void SetCtrl(uint8_t ctrlnum, uint8_t bits);
	void ClrCtrl(uint8_t ctrlnum, uint8_t bits);

	// Simple wrapper to read scalar, in host byte order
	template <typename T> [[__noinline]] uint Read(uint8_t reg) {
		T tmp;
		return Read(reg, &tmp, sizeof (T)) == sizeof(T) ? (uint)tmp : (uint)-1;
	}

	// Read status
	[[__finline]] uint8_t GetStatus() { return Read<uint8_t>(REG_STATUS); }
};


#endif // __LIS302DL_H__
