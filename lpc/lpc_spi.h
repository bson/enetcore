#ifndef __LPC_SPI_H__
#define __LPC_SPI_H__

#include "lpc_crc.h"
#include "mutex.h"

// SPI bus
class LpcSpiBus {
	volatile uint32// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.
_t* _base;


    enum {
        // (RW) Control Register 0. Selects the serial clock rate, bus
        // type, and data size.
        REG_CR0 = 0x000/4,

        // (RW) Control Register 1. Selects master/slave and other modes.
        REG_CR1 = 0x004/4,

        // (RW) Data Register. Writes fill the transmit FIFO, and
        // reads empty the receive FIFO.
        REG_DR = 0x008/4,

        // (RO) Status Register
        REG_SR = 0x00C/4,

        // (RW) Clock Prescale Register
        REG_CPSR = 0x010/4,

        // (RW) Interrupt Mask Set and Clear Register
        REG_IMSC = 0x014/4,

        // (RW) Raw Interrupt Status Register
        REG_RIS = 0x018/4,

        // (RW) Masked Interrupt Status Register
        REG_MIS = 0x01C/4,

        // (RW) SSPICR Interrupt Clear Register
        REG_ICR = 0x020/4,

        // (RW) DMA Control Register
        REG_DMACR = 0x024/4,
    };

    // Various config bits
    enum {
        CR0_DSS_FLD   = BIT0,        // Data size select; bits - 1, e.g. 0x7 for 8 bits
        CR0_FRF_SPI   = 0b00 * BIT4, // Frame fmt: SPI
        CR0_FRF_TI    = 0b01 * BIT4, // ... TI
        CR0_FRF_MW    = 0b10 * BIT4, // ... Microwire
        CR0_CPOL_H    = 0 * BIT6,    // Clk pola H (idle L)
        CR0_CPOL_L    = 1 * BIT6,    // Clk pola L (idle H)
        CR0_CPHA_LE   = 0 * BIT7,    // Clk phase leading edge
        CR0_CPHA_TE   = 1 * BIT7,    // Clk phase trailing edge

        // Serial clock rate field (8:15); serial_clk = PCLK/CPSDVSR/(SCR+1)
        CR0_SCR_FLD   = BIT8,        
        
        CR1_LBM       = BIT0,   // Loopback mode
        CR1_SSE       = BIT1,   // SSP enable
        CR1_MASTER    = 0*BIT2, // Master mode
        CR1_SLAVE     = 1*BIT2, // Slave mode
        CR1_SOD       = BIT3,   // Slave output disable

        SR_TFE        = BIT0,   // Transmit FIFO empty
        SR_TNF        = BIT1,   // Transmit FIFO not full
        SR_RNE        = BIT2,   // Receive FIFO not empty
        SR_RFF        = BIT3,   // Receive FIFO full
        SR_BSY        = BIT4,   // Busy: sending/receiving, or TX fifo not empty
    };

public:
	LpcSpiBus(uintptr_t base);

	void Init();

protected:
    friend class LpcSpiDev;

	// Recomputes prescaler
	void SetSpeed(uint freq);

	// Send byte sequence, returns byte received on last byte, or -1
	int Send(const uint8_t* s, uint len);

	// Send byte, read reply; returns -1 if no reply received
	int Read(uint8_t code = 0xff);

	// Send byte, repeating at interval, a given number of times until
	// something is received; if so, return it.  Returns -1 if nothing
	// was received.
	int ReadReply(uint interval, uint num_tries, uint8_t code = 0xff);

	// Read a given number of bytes, appending to buffer, computing CRC on the fly.
	// Returns false if we had an error during the receive
	bool ReadBuffer(void* buffer, uint len, CrcCCITT* crc = NULL);

    // Wait until idle
    void WaitIdle();
};


// Simple SPI device
// Devices are associated with a bus and are distinguished by the output
// used for SSEL.
// While each device has its own speed, currently polarity and clock phase
// aren't settable per device.
// This is probably not hardware specific, but might be bus specific.

class LpcSpiDev {
	LpcSpiBus& _bus;
	Output* _ssel;				// SSEL output for this device or NULL if none
	uint _speed;				// Speed setting
	bool _selected;				// Tracks whether currently selected
public:
	LpcSpiDev(LpcSpiBus& bus);
	
	// Init is currently a no-op
	[[__finline]] void Init() { }
	[[__finline]] void SetSSEL(Output* ssel) { _ssel = ssel; }
	void SetSpeed(uint freq);

	void Select();
	void Deselect();

	// These are delegated from bus - see SPI declaration for comments
	[[__finline]] uint8_t Send(const uint8_t* s, uint len) { return _bus.Send(s, len); }
	[[__finline]] uint8_t Read(uint8_t code = 0xff) { return _bus.Read(code); }
	[[__finline]] uint8_t ReadReply(uint interval, uint num_tries, uint8_t code = 0xff) {
		return _bus.ReadReply(interval, num_tries, code);
	}
	[[__finline]] bool ReadBuffer(void* buffer, uint len, CrcCCITT* crc = NULL) {
		return _bus.ReadBuffer(buffer, len, crc);
	}
};


#endif // __SPI_H__