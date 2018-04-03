#include <stdint.h>

// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetcore.h"
#include "thread.h"
#include "mutex.h"


LpcSpiBus::LpcSpiBus(uintptr_t base) {
    _base = (volatile uint32_t*)base;
}


void LpcSpiBus::Init() {
}


void LpcSpiBus::SetSpeed(uint freq) {
    assert(freq <= PCLK/12);

    const uint scaler = PCLK / freq;
    uint prescaler = scaler & 0xff;
    uint scr = scaler / 0x100;

    if (prescaler == 0xff){
        --prescaler;
        ++scr;
    }

    assert(prescaler >= 2);

    // 0x7 - 8 bit transfer, SPI, CLK H (idle L), leading edge clk
    _base[REG_CR0] = (7 * CR0_DSS_FLD) | CR0_FRF_SPI | CR0_CPOL_H | CR0_CPHA_LE 
        | ((scr - 1) * CR0_SCR_FLD);
    
    _base[REG_CR1] = CR1_SSE | CR1_MASTER;
    _base[REG_CPSR] = prescaler;

    DMSG("SpiBus: Prescaler = %u, scr = %u, clock = %ukHz",
         prescaler, scr, PCLK/1000/prescaler/(scr+1));
}


void LpcSpiBus::WaitIdle() {
    // Wait until idle
    while (!(_base[REG_SR] & SR_BSY))
        ;

    // Wait for transmitter FIFO empty
    while (!(_base[REG_SR] & SR_TFE))
        ;

    // Drain RX FIFO if it has data
    while (_base[REG_SR] & SR_RNE)
        (void)_base[REG_DR];
}


int LpcSpiBus::Read(uint8_t code) {
    WaitIdle();

    // Clock out code to facilitate read
    _base[REG_DR] = code;

    // Wait for TX to finish
    while (!(_base[REG_SR] & SR_TFE))
        ;

    // Check if something arrived in RX
    if (!(_base[REG_SR] & SR_RNE))
        return -1;

    return _base[REG_DR];
}


int LpcSpiBus::Send(const uint8_t* s, uint len) {
    int tmp = 0;
    while (len--)
        tmp = Read(*s++);

    return tmp;
}


int LpcSpiBus::ReadReply(uint interval, uint num_tries, uint8_t code) {
    while (num_tries--) {
        const int tmp = Read(code);
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

    WaitIdle();

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
        _bus.SetSpeed(_speed);

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


void LpcSpiDev::SetSpeed(uint freq) {
    _speed = freq;

    if (_selected)
        _bus.SetSpeed(_speed);
}
