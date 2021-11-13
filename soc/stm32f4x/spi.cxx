// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#include <stdint.h>
#include "core/enetcore.h"
#include "core/thread.h"
#include "core/util.h"
#include "soc/stm32f4x/spi.h"


void Stm32SpiBus::DmaTxComplete() {
    _dma.ReleaseTx(this);

    if (!_rx._active && _dev)
        Thread::WakeAll((void*)_dev);
}

void Stm32SpiBus::DmaRxComplete() {
    _dma.ReleaseRx(this);

    if (!_tx._active && _dev)
        Thread::WakeAll((void*)_dev);
}

void Stm32SpiBus::DmaEnableTx() { reg(Register::CR2) |= BIT(TXDMAEN); }
void Stm32SpiBus::DmaDisableTx() { reg(Register::CR2) &= ~BIT(TXDMAEN); }
void Stm32SpiBus::DmaEnableRx() { reg(Register::CR2) |= BIT(RXDMAEN); }
void Stm32SpiBus::DmaDisableRx() { reg(Register::CR2) &= ~BIT(RXDMAEN); }

void Stm32SpiBus::Configure(uint32_t mode, uint32_t freq) {
    // Ignore if already configured correctly
    if (mode == _mode && freq == _speed)
        return;

    const uint32_t psc = _busclk/freq;
    assert(psc <= 256);
    assert(psc >= 2);

    uint32_t div = 256 >> 1;
    uint16_t br;
    for (br = 7; div > psc && br > 0; --br)
        div >>= 1;

    // 0x7 - 8 bit transfer, SPI, CPOLA and CPHA according to SPI mode
    const uint32_t cpol = (mode == 2 || mode == 3) ? BIT(CPOL) : 0;
    const uint32_t cpha = (mode == 1 || mode == 2) ? BIT(CPHA) : 0;

    while (reg(Register::SR) & BIT(BSY))
        ;

    reg(Register::CR1) = cpol | cpha | BIT(MSTR) | (br << BR)
        | BIT(SSM) | BIT(SSI) | BIT(SPE);
    reg(Register::CR2) = 0;

    const uint32_t tmp = reg(Register::DR);

    DMSG("SpiBus: freq=%uHz, speed %uHz, prescaler = %u, br = %u",
         freq, _busclk/(1 << (br + 1)), psc, br);

    _mode = mode;
    _speed = freq;
}


uint8_t Stm32SpiBus::Send(uint8_t code) {
    while ((reg(Register::SR) & BIT(TXE)) == 0)
        ;

    reg(Register::DR) = code;

    while (reg(Register::SR) & BIT(BSY))
        ;

    return reg(Register::DR);
}


void Stm32SpiBus::Transact(const uint8_t* txbuf, uint32_t txlen,
                           uint8_t* rxbuf, uint32_t rxlen) {
    while (reg(Register::SR) & BIT(BSY))
        ;

    // Empty RX DR, clear RXNE
    const uint8_t tmp = reg(Register::DR);

    reg(Register::CR1) &= ~BIT(SPE);
    reg(Register::CR2) |= BIT(TXDMAEN) | BIT(RXDMAEN);
    reg(Register::CR1) |= BIT(MSTR) | BIT(SPE);

    _dma.AssignRxTx(this);
    _dma.Receive(this, (void*)rxbuf, rxlen);
    _dma.Transmit(this, (const void*)txbuf, txlen, true);
}


void Stm32SpiBus::Acquire(Stm32SpiDev* dev) {
    Thread::IPL G(IPL_SPI-1);
    
    while (_dev_count && _dev != dev)
        Thread::WaitFor(this);

    ++_dev_count;
    _dev = dev;
}


void Stm32SpiBus::Release(Stm32SpiDev *dev) {
    Thread::IPL G(IPL_SPI-1);

    assert(_dev == dev);
    
    if (!--_dev_count) {
        Thread::WakeSingle(this);
        _dev = NULL;
    }
}


Stm32SpiDev::Stm32SpiDev(Stm32SpiBus& bus) :
    _bus(bus),
    _ssel(NULL),
    _speed(100000),
    _selected(false) {
}

Stm32SpiDev::Stm32SpiDev(Stm32SpiBus& bus, Output* ssel) :
    _bus(bus),
    _ssel(ssel),
    _speed(100000),
    _selected(false) {
}

void Stm32SpiDev::Select() {
    if (!_selected)  {
        _bus.Configure(_mode, _speed);

        if (_ssel)
            _ssel->Raise();

        _selected = true;
    }
}


void Stm32SpiDev::Deselect() {
    if (_selected) {
        if (_ssel)
            _ssel->Lower();

        _selected = false;
    }
}


void Stm32SpiDev::Configure(uint mode, uint32_t freq) {
    _mode = mode;
    _speed = freq;

    if (_selected)
        _bus.Configure(_mode, _speed);
}
