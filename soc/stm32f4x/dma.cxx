#include "core/enetkit.h"
#include "core/thread.h"
#include "soc/stm32f4x/dma.h"
#include "arch/armv7m/nvic.h"


void Stm32Dma::AcquireTx(Stm32Dma::Peripheral* p) {
    assert(p);

    ScopedNoInt G;
    while (_handler[p->_tx_stream] && _handler[p->_tx_stream] != p)
        Thread::WaitFor(_handler + p->_tx_stream);

    _handler[p->_tx_stream] = p;
    _is_tx[p->_tx_stream] = true;
}

bool Stm32Dma::TryAcquireTx(Stm32Dma::Peripheral* p) {
    assert(p);

    ScopedNoInt G;
    if (_handler[p->_tx_stream] && _handler[p->_tx_stream] != p)
        return false;

    _handler[p->_tx_stream] = p;
    _is_tx[p->_tx_stream] = true;
    return true;
}


void Stm32Dma::ReleaseTx(Stm32Dma::Peripheral* p) {
    assert(p == _handler[p->_tx_stream]);

    ScopedNoInt G;
    _handler[p->_tx_stream] = NULL;
    Thread::WakeSingle(_handler + p->_tx_stream);
}


void Stm32Dma::Transmit(Stm32Dma::Peripheral* p, const void* buf, uint16_t nwords, bool minc) {
    assert(nwords <= 0xffff);
    assert(nwords != 0);
    assert(buf != NULL);
    assert(p == _handler[p->_tx_stream]);
    assert(!p->_tx_active);

    volatile uint32_t& cr = s_cr(p->_tx_stream);

    NVic::SetIRQPriority(_irq[p->_tx_stream], p->_ipl);

    Thread::IPL G(p->_ipl);

    cr &= ~BIT(EN);
    ClearTCIF(p->_tx_stream);

    cr = (p->_tx_ch << CHSEL)
        | ((uint32_t)p->_prio << PL)
        | ((uint32_t)p->_word_size << MSIZE)
        | ((uint32_t)p->_word_size << PSIZE)
        | Direction::MTOP
        | (minc ? BIT(MINC) : 0)
        | BIT(TCIE);

    s_fcr(p->_tx_stream) = BIT(DMDIS);
    s_par(p->_tx_stream) = p->_dr;
    s_m0ar(p->_tx_stream) = (uint32_t)buf;
    s_ndtr(p->_tx_stream) = nwords;

    // Make it so
    p->DmaEnableTx();
    p->_tx_active = true;
    cr |= BIT(EN);
}


void Stm32Dma::AcquireRx(Stm32Dma::Peripheral* p) {
    assert(p);

    ScopedNoInt G;
    while (_handler[p->_rx_stream] && _handler[p->_rx_stream] != p)
        Thread::WaitFor(_handler + p->_rx_stream);

    _handler[p->_rx_stream] = p;
    _is_tx[p->_rx_stream] = false;
}


bool Stm32Dma::TryAcquireRx(Stm32Dma::Peripheral* p) {
    assert(p);

    ScopedNoInt G;
    if (_handler[p->_rx_stream] && _handler[p->_rx_stream] != p)
        return false;

    _handler[p->_rx_stream] = p;
    _is_tx[p->_rx_stream] = false;
    return true;
}


void Stm32Dma::ReleaseRx(Stm32Dma::Peripheral* p) {
    assert(p == _handler[p->_rx_stream]);

    ScopedNoInt G;
    _handler[p->_rx_stream] = NULL;
    Thread::WakeSingle(_handler + p->_rx_stream);
}


void Stm32Dma::Receive(Stm32Dma::Peripheral* p, void* buf, uint16_t nwords) {
    assert(nwords <= 0xffff);
    assert(nwords != 0);
    assert(buf != NULL);
    assert(p == _handler[p->_rx_stream]);
    assert(!p->_rx_active);

    volatile uint32_t& cr = s_cr(p->_rx_stream);

    NVic::SetIRQPriority(_irq[p->_rx_stream], p->_ipl);

    Thread::IPL G(p->_ipl);

    cr &= ~BIT(EN);
    ClearTCIF(p->_rx_stream);

    cr = (p->_rx_ch << CHSEL)
        | ((uint32_t)p->_prio << PL)
        | ((uint32_t)p->_word_size << MSIZE)
        | ((uint32_t)p->_word_size << PSIZE)
        | Direction::PTOM
        | BIT(MINC)
        | BIT(TCIE);

    s_fcr(p->_rx_stream) = BIT(DMDIS);
    s_par(p->_rx_stream) = p->_dr;
    s_m0ar(p->_rx_stream) = (uint32_t)buf;
    s_ndtr(p->_rx_stream) = nwords;

    // Make it so
    p->DmaEnableRx();
    p->_rx_active = true;
    cr |= BIT(EN);
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
        Peripheral* handler = dma->_handler[STREAM];
        dma->reg(IFCR) |= tcif;
        dma->s_cr(STREAM) &= ~(BIT(EN) | BIT(TCIE) | BIT(HTIE) | BIT(TEIE) | BIT(DMEIE));
        if (handler) {
            if (dma->_is_tx[STREAM]) {
                handler->_tx_active = false;
                handler->DmaDisableTx();       // Remove DMA trigger
                handler->DmaTxComplete();
            } else {
                handler->_rx_active = false;
                handler->DmaDisableRx();
                handler->DmaRxComplete();
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
