// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include "soc/stm32f4x/dma.h"


// SPI bus
class Stm32SpiBus: public Stm32Dma::Peripheral {
	const uint32_t _base;
    Stm32Dma& _dma;             // DMA controller
    uint32_t _speed = 0;        // Current bus speed
    uint32_t _busclk;           // SPI peripheral bus clock
    uint8_t  _mode = 0;         // Current SPI mode
    uint     _dev_count = 0;    // Device acquisition counter

    class Stm32SpiDev* _dev = NULL; // Device holding the bus

    enum class Register {
        CR1 = 0x00,
        CR2 = 0x04,
        SR = 0x08,
        DR = 0x0c,
        CRCPR = 0x10,
        RXCRCR = 0x14,
        TXCRCR = 0x18,
        I2SCFGR = 0x1c,
        I2SPR = 0x20,
    };

    enum {
        // CR1
        BIDMODE = 15,
        BIDIOE = 14,
        CRCEN = 13,
        CRCNEXT = 12,
        DFF = 11,
        RXONLY = 10,
        SSM = 9,
        SSI = 8,
        LSBFIRST = 7,
        SPE = 6,
        BR = 3,
        MSTR = 2,
        CPOL = 1,
        CPHA = 0,

        // CR2
        TXEIE = 7,
        TXNEIE = 6,
        ERRIE = 5,
        FRF = 4,
        SSOE = 2,
        TXDMAEN = 1,
        RXDMAEN = 0,

        // SR
        FRE = 8,
        BSY = 7,
        OVR = 6,
        MODF = 5,
        CRCERR = 4,
        UDR = 3,
        CHSIDE = 2,
        TXE = 1,
        RXNE = 0,

        // I2SCFGR
        I2SMOD = 11,
        I2SE = 10,
        I2SCFG = 8,
        PCMSYNC = 7,
        I2SSTD = 4,
        CKPOL = 3,
        DATLEN = 1,
        CHLEN = 0,

        // I2SPR
        MCKOE = 9,
        ODD = 8
    };

    [[__finline, __optimize]]
    volatile uint16_t& reg(Register r) {
        return *(volatile uint16_t*)(_base + (uint32_t)r);
    }

public:
	Stm32SpiBus(uint32_t base, uint32_t busclk,
                Stm32Dma& dma, Stm32Dma::Target txtarg, Stm32Dma::Target rxtarg,
                Stm32Dma::Priority dmaprio, uint8_t dmaipl)
        : Peripheral(base + (uint32_t)Register::DR, txtarg, rxtarg),
          _base(base),
          _dma(dma),
          _busclk(busclk) {
        _word_size = Stm32Dma::WordSize::BYTE;
        _prio = dmaprio;
        _ipl = dmaipl;
        _tx._active = false;
        _rx._active = false;
    }

protected:
    friend class Stm32SpiDev;

	// Recomputes prescaler.  Mode is SPI mode.
	void Configure(uint32_t mode, uint32_t freq);

	// Send single byte
	uint8_t Send(uint8_t code);

    // Bus transaction.  Send txbuf, receiving rxbuf.
    void Transact(const uint8_t* txbuf, uint32_t txlen,
                  uint8_t* rxbuf, uint32_t rxlen);

    // Device bus acquire
    void Acquire(class Stm32SpiDev* dev);

    // Device bus release
    void Release(class Stm32SpiDev* dev);

private:
    // DMA TX complete
    void DmaTxComplete();

    // DMA RX complete
    void DmaRxComplete();

    // Enable disable DMA (attach, detach trigger)
    void DmaEnableTx();
    void DmaDisableTx();
    void DmaEnableRx();
    void DmaDisableRx();
};


// Simple SPI device
// Devices are associated with a bus and are distinguished by the output
// used for SSEL.
// While each device has its own speed, currently polarity and clock phase
// aren't settable per device.
// This is probably not hardware specific, but might be bus specific.

class Stm32SpiDev {
	Stm32SpiBus& _bus;
	Output*      _ssel; // SSEL output for this device or NULL if none
	uint32_t     _speed;        // Speed setting
    uint8_t      _mode;         // SPI mode to use for device (0-3)
	bool         _selected;     // Tracks whether currently selected

public:
	Stm32SpiDev(Stm32SpiBus& bus);
	Stm32SpiDev(Stm32SpiBus& bus, Output* ssel);
	
	// Init is currently a no-op
    void Init() { }
    void SetSSEL(Output* ssel) { _ssel = ssel; }
	void Configure(uint mode, uint32_t freq);

    void Acquire() { _bus.Acquire(this); }
    void Release() { _bus.Release(this); }

    // Assert, deassert SS on _ssel
	void Select();
	void Deselect();

	// These are delegated from bus - see SPI declaration for comments
    void Send(const uint8_t code) { _bus.Send(code); }

    void Transact(const uint8_t* txbuf, uint32_t txlen,
                  uint8_t* rxbuf, uint32_t rxlen) {
        _bus.Transact(txbuf, txlen, rxbuf, rxlen);
    }

    class AcquireBus {
        Stm32SpiDev& _dev;
    public:
        AcquireBus(Stm32SpiDev& dev)
            : _dev(dev) {
            dev.Acquire();
        }

        ~AcquireBus() {
            _dev.Release();
        }
    };
};


#endif // __SPI_H__
