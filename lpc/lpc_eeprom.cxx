// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "lpc_eeprom.h"
#include "thread.h"
#include "mutex.h"


void LpcEeprom::Init() {
    const uint32_t clkdiv = CCLK / 375000 - 1; // Needs to run at 385kHz
    assert_bounds(clkdiv <= 0xffff);

    _base[REG_CLKDIV] = clkdiv;

    uint8_t p3 = CCLK * 15 / (1000000000 / 16); // 16x wait states for 15ns
    p3 = p3 & 0xf ? p3 / 16 : p3 / 16 - 1;      // Round up, minus one

    uint8_t p2 = CCLK * 55ULL / (1000000000 / 16); // For 55ns
    p2 = p2 & 0xf ? p2 / 16 : p2 / 16 - 1;

    uint8_t p1 = CCLK * 35ULL / (1000000000 / 16); // For 35ns
    p1 = p1 & 0xf ? p1 / 16 : p1 / 16 - 1;

    DMSG("EEPROM Wait states: P3=%d, P2=%d, P1=%d", p3, p2, p1);

    _base[REG_WSTATE] = p3 + (p2 << 8) + (p3 << 16);

    _base[REG_INTENSET] = BIT26 | BIT28; // Enable interrupts

    // Power down
    _base[REG_PWRDWN] = BIT0;
}

void LpcEeprom::Acquire(LpcEeprom::State state) {
    {
        Mutex::Scoped L(_lock);

        while (_state != State::IDLE) {
            _lock.Unlock();
            Thread::WaitFor(this);
            _lock.Lock();
        }

        // Ours now
        _state = state;
    }

    // Power up
    _base[REG_PWRDWN] = 0;

    Thread::Delay(100);  // Power-up delay
}


void LpcEeprom::Write(uint16_t addr, const void* block, uint16_t len) {
    assert_bounds(addr + len < EEPROM_SIZE);
    assert((addr & 63) == 0);

    Acquire(State::WRITE);

    _pos    = (uint8_t*)block;
    _remain = (len + 1) & ~1;
    _addr   = addr;
    _page   = addr;

    if (_remain) {
        Thread::IPL G(IPL_EEPROM-1);

        _base[REG_ADDR] = _addr;
        _base[REG_CMD]  = CMD_WRITE_16;

        WriteFill();

        while (_state != State::WRITE_DONE)
            Thread::WaitFor(this);
    }

    _state = State::IDLE;
}

void LpcEeprom::Read(uint16_t addr, void* block, uint16_t len) {
    assert_bounds(addr + len < EEPROM_SIZE);

    Acquire(State::READ);

    _pos = (uint8_t*)block;
    _remain = len;
    _addr = addr;
    
    // Start read
    _base[REG_ADDR] = addr;
    if (_remain >= 2)
        _base[REG_CMD] = CMD_READ_16 | CMD_RDPREFETCH;
    else
        _base[REG_CMD] = CMD_READ_8;

    Thread::IPL G(IPL_EEPROM-1);

    while (_state != State::READ_DONE)
        Thread::WaitFor(this);

    _state = State::IDLE;
}

void LpcEeprom::WriteFill() {
    assert(_state == State::WRITE);

    // If the page register is full or we're out of data, program it
    if (_page == 64 || !_remain) {
        _state = State::PROG;
        _base[REG_ADDR] = _page;
        _base[REG_CMD]  = CMD_PROGRAM; // Erase/program page
        _page += 64;
    } else if (_remain) {
        _base[REG_WDATA] = *(const uint16_t*)_pos;

        _addr   += 2;
        _pos    += 2;
        _remain -= 2;
    }
}

void LpcEeprom::HandleInterrupt() {
    bool wake = false;

    switch (_state) {
    case State::PROG:
        if (_base[REG_INTSTAT] & BIT28) {
            _base[REG_INTSTATCLR] = BIT28;

            // Done programming a page
            if (!_remain) {
                // All done
                _state = State::WRITE_DONE;
                wake = true;

                // Power down
                _base[REG_PWRDWN] = BIT0;
            } else {
                // Continue writing
                _state = State::WRITE;
                _base[REG_ADDR] = _page;
                _base[REG_CMD]  = CMD_WRITE_16;

                while (_base[REG_INTSTATCLR] & BIT28)
                    WriteFill();
            }
        }
        break;

    case State::WRITE:
        while (_state == State::WRITE && (_base[REG_INTSTAT] & BIT26)) {
            WriteFill();
        }
        break;

    case State::READ:
        while (_base[REG_INTSTAT] & BIT26) {
            if (_remain >= 2) {
                *(uint16_t*)_pos = _base[REG_RDATA];
                _pos += 2;
                _addr += 2;
                if ((_remain -= 2) == 1) {
                    _base[REG_ADDR] = _addr;
                    _base[REG_CMD]  = CMD_READ_8;
                }
            } else if (_remain) {
                *_pos++ = _base[REG_RDATA];
                ++_addr;
                --_remain;

                assert(!_remain);
            }

            if (!_remain) {
                wake = true;
                _state = State::READ_DONE;
                // Power down
                _base[REG_PWRDWN] = BIT0;
                break;
            }
        }
        break;

    default: ;
    }

    if (wake)
        Thread::WakeAll(this);

    _base[REG_INTSTATCLR] = BIT26 | BIT28;
}

void LpcEeprom::Interrupt(void* token) {
    LpcEeprom* e = (LpcEeprom*)token;
    e->HandleInterrupt();
}

