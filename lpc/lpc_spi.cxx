#include <stdint.h>

// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetcore.h"
#include "thread.h"
#include "mutex.h"


LpcSpiBus::LpcSpiBus(uintptr_t base)
    : _base((volatile uint32_t*)base),
      _speed(0),
      _mode(0) {
}


void LpcSpiBus::Init() {
}


void LpcSpiBus::Configure(uint mode, uint freq) {
    // Ignore if already configured correctly
    if (mode == _mode && freq == _speed)
        return;

    assert(freq <= PCLK/12);

    const uint scaler = PCLK / freq;
    uint prescaler = scaler;
    uint scr = 1;

    if (scaler > 254) {
        prescaler = scaler / 2;
        scr = scaler / 256 + 1;
    }

    assert(prescaler >= 2);

    // 0x7 - 8 bit transfer, SPI, CPOLA and CPHA according to SPI mode
    const uint8_t cpol = (mode & 2) ? CR0_CPOL_L : CR0_CPOL_H;
    const uint8_t cpha = (mode & 1) ? CR0_CPHA_TE : CR0_CPHA_LE;

    _base[REG_CR0] = (7 * CR0_DSS_FLD) | CR0_FRF_SPI | cpol | cpha 
        | ((scr - 1) * CR0_SCR_FLD);
    
    _base[REG_CR1] = CR1_SSE | CR1_MASTER;
    _base[REG_CPSR] = prescaler;

    DMSG("SpiBus: freq=%ukHz, prescaler = %u, scr = %u, clock = %ukHz",
         freq/1000, prescaler, scr, PCLK/prescaler/scr/1000);

    _mode = mode;
    _speed = freq;
}


void LpcSpiBus::WaitIdle() {
    // Wait until idle
    while (_base[REG_SR] & SR_BSY)
        ;

    // Wait for transmitter FIFO empty
    while (!(_base[REG_SR] & SR_TFE))
        ;

    // Drain RX FIFO if it has data
    while (_base[REG_SR] & SR_RNE)
        (void)_base[REG_DR];
}


inline int LpcSpiBus::SendRead(uint8_t code) {
    WaitIdle();

    // Clock out 0xff to facilitate read
    _base[REG_DR] = code;

    // Wait for TX to finish
    while (!(_base[REG_SR] & SR_TFE))
        ;

    // Check if something arrived in RX
    if (!(_base[REG_SR] & SR_RNE))
        return -1;

    return _base[REG_DR];
}


int LpcSpiBus::Read() {
    return SendRead(0xff);
}


inline void LpcSpiBus::Send(const uint8_t* s, uint len) {
    // Drain any remaining data the device is trying to send
    while (Read() != -1)
        ;

    while (len--) {
        _base[REG_DR] = *s++;

        // Wait for TX to finish
        while (!(_base[REG_SR] & SR_TFE))
            ;
    }
}

void LpcSpiBus::Send(uint8_t s) {
    Send(&s, 1);
}


int LpcSpiBus::ReadReply(uint interval, uint num_tries) {
    while (num_tries--) {
        const int tmp = Read();
        if (tmp != -1) 
            return tmp;

        if (interval)
            Thread::Delay(interval);
    }

    return -1;
}


bool LpcSpiBus::ReadBuffer(void* buffer, uint len, CrcCCITT* crc) {
    if (!len)
        return true;

    uint8_t* p = (uint8_t*)buffer;
    bool ok = true;

    _base[REG_DR] = 0xff;

    while (len--) {
        while (_base[REG_SR] & SR_BSY)
            ;

        if (!(_base[REG_SR] & SR_RNE))
            ok = false;

        const uint8_t tmp = _base[REG_DR];
        if (len)
            _base[REG_DR] = 0xff;

        // Then during SPI clocking store the value read and update
        // running Crc
        *p++ = tmp;
        crc->Update(tmp);
    }

    return ok;
}


LpcSpiDev::LpcSpiDev(LpcSpiBus& bus) :
    _bus(bus),
    _ssel(NULL),
    _speed(100000),
    _selected(false) {
}

void LpcSpiDev::Select() {
    if (!_selected)  {
        _bus.Configure(_mode, _speed);

        if (_ssel)
            _ssel->Raise();

        _selected = true;
    }
}


void LpcSpiDev::Deselect() {
    if (_selected) {
        if (_ssel)
            _ssel->Lower();

        _selected = false;
    }
}


void LpcSpiDev::Configure(uint mode, uint freq) {
    _mode = mode;
    _speed = freq;

    if (_selected)
        _bus.Configure(_mode, _speed);
}
