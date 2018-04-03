// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "thread.h"
#include "board.h"
#include "nvic.h"


extern NVic _nvic;

LpcI2cBus::LpcI2cBus(uintptr_t base, uint8_t irq) :
	_base((volatile uint32_t*)base),
	_state(STATE_IDLE),
    _irq(irq),
	_slave(0),
	_bufptr(NULL),
	_buflen(0),
	_acquired(false)
{
}


void LpcI2cBus::Init()
{
	_base[REG_CONSET] = CON_EN;
}


void LpcI2cBus::SetSpeed(uint hz)
{
	const uint div = max<uint>(PCLK / hz, 8);

    assert(div <= 0x1ffff);

	Mutex::Scoped L(_lock);

	// If divider is odd, add extra clock to low
	_base[REG_SCLH] = div >> 1;
	_base[REG_SCLL] = (div >> 1) + (div & 1);
}


void LpcI2cBus::Interrupt(void *token)
{
    LpcI2cBus* bus = (LpcI2cBus*)token;

    bus->HandleInterrupt();
}


void LpcI2cBus::HandleInterrupt()
{
	switch (_state) {
	case STATE_MXMIT: {
		switch (_base[REG_STAT]) {
		case 0x20:				// SLA+W NACK
		case 0x38:				// Lost bus
			// Repeat start
			_base[REG_CONCLR] = CON_AA;
			_base[REG_CONSET] = CON_STA;
			break;
		case 0x08:				// Sent Start
		case 0x10:				// Sent repeated Start
			_base[REG_DAT] = _slave & ~SLA_R;
			_base[REG_CONCLR] = CON_AA|CON_STA;
			_base[REG_CONSET] = 0;
			break;
		case 0x30:				// Data byte NACK
			// Retransmit last byte
			assert(_pos);
			--_pos;
			// fallthru
		case 0x18:				// SLA+W sent, ACK recvd
		case 0x28:				// Data byte ACK
			// Send next data byte
			if (_pos < _buflen) {
				_base[REG_DAT] = _bufptr[_pos++];
				_base[REG_CONCLR] = CON_STA;
				_base[REG_CONSET] = CON_AA;
			} else {
				_base[REG_CONCLR] = _final && _final_nak ? CON_STA : CON_STA|CON_AA;
				_base[REG_CONSET] = _final ? CON_STO : 0;
				_state = STATE_DONE;
			}
			break;
		}
		break;
	}
	case STATE_MRECV: {
		switch (_base[REG_STAT]) {
		case 0x38:				// Lost bus
		case 0x48:				// SLA+R sent, got NACK
			// Repeat start
			_base[REG_CONCLR] = CON_AA;
			_base[REG_CONSET] = CON_STA;
			break;
		case 0x08:				// Sent Start
		case 0x10:				// Sent repeated Start
			_base[REG_DAT] = _slave | SLA_R;
			_base[REG_CONCLR] = CON_AA;
			_base[REG_CONSET] = CON_STA;
			break;
		case 0x40:				// Sent SLA+R
			// Send ack
			_base[REG_CONCLR] = CON_STA;
			_base[REG_CONSET] = CON_AA;
			break;
		case 0x50:				// Recevied byte, ACK
			if (_pos < _buflen) {
				_bufptr[_pos++] = _base[REG_DAT];
				_base[REG_CONCLR] = CON_STA;
				_base[REG_CONSET] = CON_AA;
			} else {
				_base[REG_CONCLR] = _final && _final_nak ? CON_STA : CON_STA|CON_AA;
				_base[REG_CONSET] = _final ? CON_STO : 0;
				_state = STATE_DONE;
			}
			break;
		case 0x58:				// Data byte, NACK
			// Simply stop, returning what we received
			_base[REG_CONCLR] = _final && _final_nak ? CON_STA : CON_STA|CON_AA;
			_base[REG_CONSET] = _final ? CON_STO : 0;
			_state = STATE_DONE;
			break;
		}
		break;
	}
	case STATE_IDLE:
        break;

	case STATE_SXMIT:
	case STATE_SRECV:
		;
	}

	_base[REG_CONCLR] = CON_SI;	// Clear SI flag

	if (_state == STATE_DONE)
		_change.Signal();
}


void LpcI2cBus::Acquire()
{
	Mutex::Scoped L(_lock);

	while (_acquired)  
        _change.Wait(_lock);
    
	_acquired = true;
}


void LpcI2cBus::Release()
{
	Mutex::Scoped L(_lock);

	_acquired = false;
	_change.Signal();
}


uint LpcI2cBus::Cycle(uint8_t slave, uint8_t* buf, uint len, uint state0)
{
	Mutex::Scoped L(_lock);

	while (_state != STATE_IDLE)
		_change.Wait(_lock);
	
	_state = state0;

	_slave = slave;
	_bufptr = buf;
	_buflen = len;
	_pos = 0;
	_final = false;
	_final_nak = true;			// Default to NAK after last receive
	
	// Send Start to kick off state machine
	_base[REG_CONCLR] = CON_AA|CON_SI;
	_base[REG_CONSET] = CON_STA;

	while (_state != STATE_DONE)
		_change.Wait(_lock);

	_state = STATE_IDLE;
	_change.Signal();

	return _pos;
}


void LpcI2cBus::Write(uint8_t slave, const uint8_t* buf, uint len)
{
	Cycle(slave, (uint8_t*)buf, len, STATE_MXMIT);
}


uint LpcI2cBus::Read(uint8_t slave, uint8_t* buf, uint len)
{
	return Cycle(slave, buf, len, STATE_MRECV);
}


LpcI2cDev::LpcI2cDev(LpcI2cBus& bus, uint8_t slave) :
	_bus(bus),
	_slave(slave)
{
}
