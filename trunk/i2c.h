#ifndef __I2C_H__
#define __I2C_H__


// I2C bus
class I2cBus {

	// Bits for I2C_CONSET/CONCLR.
	enum {
		CON_AA  = 0b100,
		CON_SI  = 0b1000,
		CON_STO = 0b10000,
		CON_STA = 0b100000,
		CON_EN  = 0b1000000
	};

	enum { SLA_R = 1 };

	enum Mode {
		STATE_IDLE = 0,			// Idle
		STATE_MXMIT,			// Master transmit
		STATE_MRECV,			// Master receive
		STATE_SXMIT,			// Slave transmit
		STATE_SRECV,			// Slave receive
		STATE_DONE				// Done
	};

	volatile uint32_t* _base;
	Mutex _lock;
	CondVar _change;			// Signals changes
	uint8_t _state;				// Current state, STATE_xxx

	// Current request
	uint8_t _slave;		   // Slave address
	uint8_t* _bufptr;	   // Byte string to send/receive
	uint8_t _buflen;	   // Number of bytes left
	uint8_t _pos;		   // Next byte to send/receive
	bool _final:1;		   // Request is final (issue STO after)
	bool _final_nak:1;	   // End last receive with NAK rather than ACK prior to Stop
	bool _acquired:1;	   // Bus is acquired

public:
	I2cBus(uintptr_t base);

	void Init();
	void SetSpeed(uint hz);

	static void Interrupt() __irq __naked;

	void HandleInterrupt();

	// Acquire/release bus
	void Acquire();
	void Release();

	// Write buffer to slave
	void Write(uint8_t slave, const uint8_t* buf, uint len);

	// Read buffer from slave
	// Returns actual number of bytes received
	uint Read(uint8_t slave, uint8_t* buf, uint len);

	// Set final flag - read/write is last is series and will be
	// finished with Stop
	__force_inline void Final() { _final = true; }

	// Indicate final transaction ends with ACK rather than the default NAK
	__force_inline void FinalAck() { _final_nak = false; }

private:
	// Master cycle (send/recv)
	uint Cycle(uint8_t slave, uint8_t* buf, uint len, uint state0);
};


extern I2cBus _i2c0;


// I2C device
class I2cDev {
	I2cBus& _bus;
	uint8_t _slave;

public:
	I2cDev(I2cBus& bus, uint8_t slave);

	__force_inline void AcquireBus() { _bus.Acquire(); }
	__force_inline void ReleaseBus() { _bus.Release(); }
	__force_inline void Final() { _bus.Final(); }
	__force_inline void FinalAck() { _bus.FinalAck(); }
	__force_inline void Write(const uint8_t* buf, uint len) { _bus.Write(_slave, buf, len); }
	__force_inline uint Read(uint8_t* buf, uint len) { return _bus.Read(_slave, buf, len); }
};

#endif // __I2C_H__
