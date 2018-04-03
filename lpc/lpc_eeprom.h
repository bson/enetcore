// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _LPC_EEPROM_H_
#define _LPC_EEPROM_H_

#include <stdint.h>
#include "lpc.h"
#include "deque.h"
#include "mutex.h"
#include "bits.h"


class LpcEeprom {
    Mutex _lock;
    volatile uint32_t* _base;

    enum {
        // (R/W) EEPROM command register
        REG_CMD = 0x080/4,

        // (R/W) EEPROM address register
        REG_ADDR = 0x084/4,

        // (WO ) EEPROM write data register
        REG_WDATA = 0x088/4,

        // (RO) EEPROM read data register
        REG_RDATA = 0x08c/4,

        // (R/W) EEPROM wait state register
        REG_WSTATE = 0x090/4,

        // (R/W) EEPROM clock divider register
        REG_CLKDIV = 0x094/4,

        // (R/W) EEPROM power-down register
        REG_PWRDWN = 0x098/4,

        // (RO) EEPROM interrupt status 
        REG_INTSTAT = 0xfe0/4,

        // (WO) EEPROM interrupt status clear 
        REG_INTSTATCLR = 0xfe8/4,

        // (WO) EEPROM interrupt status set 
        REG_INTSTATSET = 0xfec/4,

        // (RO) EEPROM interrupt enable 
        REG_INTEN = 0xfe4/4,

        // (WO) EEPROM interrupt enable clear 
        REG_INTENCLR = 0xfd8/4,

        // (WO) EEPROM interrupt enable set
        REG_INTENSET = 0xfdc/4
    };

    // Command codes 
    enum {
        CMD_READ_8   = 0,          // Read 8 bit
        CMD_READ_16  = 1,          // Read 16 bit
        CMD_READ_32  = 2,          // Read 32 bit (two 16 bit ops with bus stall!)
        CMD_WRITE_8  = 3,          // Write 8 bit
        CMD_WRITE_16 = 4,          // Write 16 bit
        CMD_WRITE_32 = 5,          // Write 32 bit
        CMD_PROGRAM  = 6,          // Erase/program page buffer
        CMD_RDPREFETCH = BIT3      // Prefetch reads and auto inc address
    };

    enum class State: uint8_t {
        IDLE = 0,
        WRITE,            // Writing
        WRITE_DONE,       // Write process completed
        PROG,             // Programming
        READ,             // Reading
        READ_DONE,        // Reading complete
    };
    
    EventObject _eventob;      // Where we signal state changes

    State _state;
    uint8_t _irq;

    // While in read or write
    uint8_t* _pos;              // Next position in buffer to read from or write to
    uint16_t _remain;           // Number of bytes remaining to read or write
    uint16_t _addr;             // EEPROM read or write address
    uint16_t _page;             // Number of bytes in the page buffer


public:
    LpcEeprom(uintptr_t base, uint8_t irq)
        : _base((volatile uint32_t*)base),
          _state(State::IDLE),
          _irq(irq)
    { }

    // Initialize
    void Init();

    uint8_t irq() const { return _irq; }

    // Write block of data.  Returns true on success.  The block is
    // rounded up to an even number of half words.
    void Write(uint16_t addr, const void* block, uint16_t len);

    // Read a block of data.  Returns true on success.
    void Read(uint16_t addr, void* block, uint16_t len);

	// Interrupt handler
	static void Interrupt(void* token);
private:
	void HandleInterrupt();

    // Atomically acquire and put in new state
    void Acquire(State state);

    // Write to page buffer
    void WriteFill();

private:
    LpcEeprom();
    LpcEeprom(const LpcEeprom&);
    LpcEeprom& operator=(const LpcEeprom&);
};


#endif // _LPC_EEPROM_H_
