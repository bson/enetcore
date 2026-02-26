// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#include "core/enetkit.h"
#include "core/thread.h"
#include "soc/stm32h7x/dma.h"
#include "arch/armv7m/nvic.h"

// XXX currently maps DMA1 to DMAMUX1
// Add transparent support for DMA (ch 8-15) by increating size of Assignment _stream
// Add mapping with DMAMUX2 and BDMA, by encoding it as the high bit of Target

Stm32Dma::Stm32Dma(uint32_t base, const uint16_t* irqs)
    : _base(base),
      _irq(irqs)
{
    memset(_assignment, 0, sizeof _assignment);
    memset(_is_tx, 0, sizeof _is_tx);
};

bool Stm32Dma::TryAssign(Stm32Dma::Peripheral* p,
                         Stm32Dma::Peripheral::Assignment& asn,
                         bool istx) {
    assert(p);
    assert(asn._target < Target::NUM_TARGET);
    assert(!asn._active);
    assert(p->_stream_cand != 0);

    ScopedNoInt G;
    uint32_t cand = p->_stream_cand;
    while (cand && (cand & 0xff) != 0xff) {
        const uint8_t stream = cand;

        assert(_assignment[stream] != p); // Not watertight, but a good assertion

        if (!_assignment[stream]) {
            _assignment[stream] = p;
            _is_tx[stream] = istx;
            asn._stream = stream;
            return true;
        }

        cand >>= 8;
    }

    return false;
}


bool Stm32Dma::TryAssignTx(Stm32Dma::Peripheral* p) {
    return TryAssign(p, p->_tx, true);
}


void Stm32Dma::AssignTx(Stm32Dma::Peripheral* p) {
    assert(p);

    ScopedNoInt G;
    while (!TryAssignTx(p))
        Thread::WaitFor(_assignment);
}


void Stm32Dma::ReleaseTx(Stm32Dma::Peripheral* p) {
    assert(p == _assignment[p->_tx._stream]);

    volatile uint32_t& mux_rgxcr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_RG1CR + 0x04 * p->_rx._stream);

    ScopedNoInt G;

    mux_rgxcr = 0;

    _assignment[p->_tx._stream] = NULL;
    Thread::WakeAll(_assignment);
}


void Stm32Dma::Transmit(Stm32Dma::Peripheral* p, const void* buf, uint16_t nwords, bool minc) {
    assert(nwords <= 0xffff);
    assert(nwords != 0);
    assert(buf != NULL);
    assert(!p->_tx._active);

    const uint32_t stream = p->_tx._stream;
    assert(p == _assignment[stream]);

    volatile uint32_t& cr = s_cr(stream);

    NVic::SetIRQPriority(_irq[stream], p->_ipl);

    Thread::IPL G(p->_ipl);

    cr &= ~BIT(EN);
    ClearTCIF(stream);

    cr = ((uint32_t)p->_prio << PL)
        | ((uint32_t)p->_word_size << MSIZE)
        | ((uint32_t)p->_word_size << PSIZE)
        | Direction::MTOP
        | (minc ? BIT(MINC) : 0)
        | BIT(TCIE);

    s_fcr(stream) = BIT(DMDIS);
    if (p->_trbuff)
        s_fcr(stream) |= BIT(TRBUFF);
    s_par(stream) = p->_tx._dr;
    s_m0ar(stream) = (uint32_t)buf;
    s_ndtr(stream) = nwords;

    // Make it so
    p->DmaEnableTx();
    p->_tx._active = true;

    // Configure DMAMUX
    volatile uint32_t& mux_cxcr = *(volatile uint32_t*)(DMAMUX1_BASE + 0x04 * stream);
    volatile uint32_t& mux_cfr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_CFR);
    volatile uint32_t& mux_rgxcr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_RG1CR + 0x04 * stream);
    volatile uint32_t& mux_rgcfr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_RGCFR);

    mux_cxcr    = 0;
    mux_cfr     = BIT(stream);
    mux_rgxcr   = (uint32_t(p->_tx._n_ops) << GNBREQ) | (0b01 << GPOL) | BIT(GE);
    mux_rgcfr   = BIT(stream);
    
    cr |= BIT(EN);
}


bool Stm32Dma::TryAssignRx(Stm32Dma::Peripheral* p) {
    return TryAssign(p, p->_rx, false);
}

void Stm32Dma::AssignRx(Stm32Dma::Peripheral* p) {
    assert(p);

    ScopedNoInt G;
    while (!TryAssignRx(p))
        Thread::WaitFor(_assignment);
}

void Stm32Dma::ReleaseRx(Stm32Dma::Peripheral* p) {
    assert(p == _assignment[p->_rx._stream]);

    volatile uint32_t& mux_rgxcr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_RG1CR + 0x04 * p->_rx._stream);

    ScopedNoInt G;

    mux_rgxcr   = 0;

    _assignment[p->_rx._stream] = NULL;
    Thread::WakeAll(_assignment);
}


void Stm32Dma::Receive(Stm32Dma::Peripheral* p, void* buf, uint16_t nwords) {
    assert(nwords <= 0xffff);
    assert(nwords != 0);
    assert(buf != NULL);
    assert(!p->_rx._active);

    const uint32_t stream = p->_rx._stream;
    assert(p == _assignment[stream]);

    volatile uint32_t& cr = s_cr(stream);

    NVic::SetIRQPriority(_irq[stream], p->_ipl);

    Thread::IPL G(p->_ipl);

    cr &= ~BIT(EN);
    ClearTCIF(stream);

    cr = ((uint32_t)p->_prio << PL)
        | ((uint32_t)p->_word_size << MSIZE)
        | ((uint32_t)p->_word_size << PSIZE)
        | Direction::PTOM
        | BIT(MINC)
        | BIT(TCIE);

    s_fcr(stream) = BIT(DMDIS);
    if (p->_trbuff)
        s_fcr(stream) |= BIT(TRBUFF);
    s_par(stream) = p->_tx._dr;
    s_m0ar(stream) = (uint32_t)buf;
    s_ndtr(stream) = nwords;

    // Make it so
    p->DmaEnableRx();
    p->_rx._active = true;

    // Configure DMAMUX
    volatile uint32_t& mux_cxcr = *(volatile uint32_t*)(DMAMUX1_BASE + 0x04 * stream);
    volatile uint32_t& mux_cfr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_CFR);
    volatile uint32_t& mux_rgxcr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_RG1CR + 0x04 * stream);
    volatile uint32_t& mux_rgcfr = *(volatile uint32_t*)(DMAMUX1_BASE + DMAMUX_RGCFR);

    mux_cxcr    = 0;
    mux_cfr     = BIT(stream);
    mux_rgxcr   = (uint32_t(p->_tx._n_ops) << GNBREQ) | (0b01 << GPOL) | BIT(GE);
    mux_rgcfr   = BIT(stream);
    
    cr |= BIT(EN);
}


void Stm32Dma::AssignRxTx(Stm32Dma::Peripheral* p) {
    if (p->_tx._target > p->_rx._target) {
        AssignTx(p);
        AssignRx(p);
    } else {
        AssignRx(p);
        AssignTx(p);
    }
}


const uint32_t Stm32Dma::stream_to_tcif[8] = {
    BIT(TCIF0), BIT(TCIF1), BIT(TCIF2), BIT(TCIF3),
    BIT(TCIF4), BIT(TCIF5), BIT(TCIF6), BIT(TCIF7)
};


// Interrupt handler
template <uint32_t STREAM, Stm32Dma::Register ISR, Stm32Dma::Register IFCR>
[[__optimize]] void Stm32Dma::Interrupt(void* token) {
    Stm32Dma* dma = (Stm32Dma*)token;

    const uint32_t tcif = stream_to_tcif[STREAM];
    if (dma->reg(ISR) & tcif) {
        Peripheral* device = dma->_assignment[STREAM];
        dma->reg(IFCR) |= tcif;
        dma->s_cr(STREAM) &= ~(BIT(EN) | BIT(TCIE) | BIT(HTIE) | BIT(TEIE) | BIT(DMEIE));
        if (device) {
            if (dma->_is_tx[STREAM]) {
                device->_tx._active = false;
                device->DmaDisableTx();
                device->DmaTxComplete();
            } else {
                device->_rx._active = false;
                device->DmaDisableRx();
                device->DmaRxComplete();
            }
        }
    }
}


// Clear TCIF bit for stream
void Stm32Dma::ClearTCIF(uint32_t stream) {
    assert(stream <= 7);

    if (stream <= 4)
        reg(Register::LIFCR) |= stream_to_tcif[stream];
    else
        reg(Register::HIFCR) |= stream_to_tcif[stream];
}


void Stm32Dma::InstallHandlers() {
    NVic::RouteIRQ(_irq[0], Stm32Dma::Interrupt<0,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::RouteIRQ(_irq[1], Stm32Dma::Interrupt<1,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::RouteIRQ(_irq[2], Stm32Dma::Interrupt<2,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::RouteIRQ(_irq[3], Stm32Dma::Interrupt<3,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::RouteIRQ(_irq[4], Stm32Dma::Interrupt<4,Register::HISR,Register::HIFCR>, IPL_DMA, this);
    NVic::RouteIRQ(_irq[5], Stm32Dma::Interrupt<5,Register::HISR,Register::HIFCR>, IPL_DMA, this);
    NVic::RouteIRQ(_irq[6], Stm32Dma::Interrupt<6,Register::HISR,Register::HIFCR>, IPL_DMA, this);
    NVic::RouteIRQ(_irq[7], Stm32Dma::Interrupt<7,Register::HISR,Register::HIFCR>, IPL_DMA, this);
}
