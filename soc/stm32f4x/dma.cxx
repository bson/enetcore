#include "core/enetkit.h"
#include "core/thread.h"
#include "soc/stm32f4x/dma.h"
#include "arch/armv7m/nvic.h"


void Stm32Dma::AcquireTx(Stm32Dma::Peripheral* p) {
    assert(p);

    ScopedNoInt G;
    while (!TryAcquireTx(p))
        Thread::WaitFor(_handler + p->_tx_stream);
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
    while (!TryAcquireRx(p))
        Thread::WaitFor(_handler + p->_rx_stream);
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


void Stm32Dma::AcquireRxTx(Stm32Dma::Peripheral* p) {
    if (p->_tx_stream > p->_rx_stream) {
        AcquireTx(p);
        AcquireRx(p);
    } else {
        AcquireRx(p);
        AcquireTx(p);
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


#define S(STREAM, CHANNEL) (uint8_t(STREAM) << 4 | uint8_t(CHANNEL))
#define E (uint8_t(Target::END))
#define T(TARGET)  ((int)Stm32Dma::Target::TARGET)

const uint8_t Stm32Dma::_target_sch[(int)Stm32Dma::Target::NUM_TARGET][Stm32Dma::NUM_MAPPING] = {
    [T(SPI3_RX)]     = { S(0,0), S(2,0), E},
    [T(SPI2_RX)]     = { S(3,0),         E, E},
    [T(SPI2_TX)]     = { S(4,0),         E, E},
    [T(SPI3_TX)]     = { S(5,0), S(7,0), E},
    [T(I2C1_RX)]     = { S(0,1), S(5,1), E},
    [T(TIM7_UP)]     = { S(2,1), S(4,1), E},
    [T(I2C1_TX)]     = { S(6,1), S(7,1), E},
    [T(TIM4_CH1)]    = { S(0,2),         E, E},
    [T(I2S3_EXT_RX)] = { S(2,2), S(0,3), E},
    [T(TIM4_CH2)]    = { S(3,2),         E, E},
    [T(I2S2_EXT_TX)] = { S(4,2),         E, E},
    [T(TIM4_UP)]     = { S(6,2),         E, E},
    [T(TIM4_CH3)]    = { S(7,2),         E, E},
    [T(TIM2_UP)]     = { S(7,3), S(1,3), E},
    [T(TIM2_CH3)]    = { S(1,3),         E, E},
    [T(I2C3_RX)]     = { S(2,3),         E, E},
    [T(I2S2_EXT_RX)] = { S(3,3),         E, E},
    [T(I2C3_TX)]     = { S(4,3),         E, E},
    [T(TIM2_CH1)]    = { S(5,3),         E, E},
    [T(TIM2_CH2)]    = { S(6,3),         E, E},
    [T(TIM2_CH4)]    = { S(7,3), S(6,3), E},
    [T(UART5_RX)]    = { S(0,4),         E, E},
    [T(USART3_RX)]   = { S(1,4),         E, E},
    [T(UART4_RX)]    = { S(4,4),         E, E},
    [T(USART3_TX)]   = { S(3,4), S(4,7), E},
    [T(UART4_TX)]    = { S(4,4),         E, E},
    [T(USART2_RX)]   = { S(5,4),         E, E},
    [T(USART2_TX)]   = { S(6,4),         E, E},
    [T(UART5_TX)]    = { S(7,4),         E, E},
    [T(TIM3_CH4)]    = { S(2,5),         E, E},
    [T(TIM3_UP)]     = { S(2,5),         E, E},
    [T(TIM3_CH1)]    = { S(4,5),         E, E},
    [T(TIM3_TRIG)]   = { S(4,5),         E, E},
    [T(TIM3_CH2)]    = { S(5,5),         E, E},
    [T(TIM3_CH3)]    = { S(7,5),         E, E},
    [T(TIM5_CH3)]    = { S(0,6),         E, E},
    [T(TIM5_UP)]     = { S(0,6), S(6,6), E},
    [T(TIM5_CH4)]    = { S(1,6), S(3,6), E},
    [T(TIM5_TRIG)]   = { S(1,6), S(3,6), E},
    [T(TIM5_CH1)]    = { S(2,6),         E, E},
    [T(TIM5_CH2)]    = { S(4,6),         E, E},
    [T(TIM6_UP)]     = { S(1,7),         E, E},
    [T(I2C2_RX)]     = { S(2,7), S(3,7), E},
    [T(DAC1)]        = { S(5,7),         E, E},
    [T(DAC2)]        = { S(6,7),         E, E},
    [T(I2C2_TX)]     = { S(7,7),         E, E},
    [T(ADC1)]        = { S(0,0), S(4,0), E},
    [T(TIM8_CH1)]    = { S(2,0), S(2,7), E},
    [T(TIM8_CH2)]    = { S(2,0), S(3,7), E},
    [T(TIM8_CH3)]    = { S(2,0), S(4,7), E},
    [T(TIM1_CH1)]    = { S(6,0), S(1,6), S(3,6)},
    [T(TIM1_CH2)]    = { S(6,0), S(2,6), E},
    [T(TIM1_CH3)]    = { S(6,0), S(6,6), E},
    [T(DCMI)]        = { S(1,1), S(7,1), E},
    [T(ADC2)]        = { S(2,1), S(3,1), E},
    [T(ADC3)]        = { S(0,2), S(1,2), E},
    [T(CRYP_OUT)]    = { S(5,2),         E, E},
    [T(CRYP_IN)]     = { S(6,2),         E, E},
    [T(HASH_IN)]     = { S(7,2),         E, E},
    [T(SPI1_RX)]     = { S(0,3), S(2,3), E},
    [T(SPI1_TX)]     = { S(3,3), S(5,3), E},
    [T(USART1_RX)]   = { S(2,4), S(5,4), E},
    [T(SDIO)]        = { S(3,4), S(6,4), E},
    [T(USART1_TX)]   = { S(7,4),         E, E },
    [T(USART6_RX)]   = { S(1,5), S(2,5), E},
    [T(USART6_TX)]   = { S(6,5), S(7,5), E},
    [T(TIM1_TRIG)]   = { S(0,6), S(4,6), E},
    [T(TIM1_CH4)]    = { S(4,6),         E, E},
    [T(TIM1_COM)]    = { S(4,6),         E, E},
    [T(TIM1_UP)]     = { S(5,6),         E, E},
    [T(TIM8_UP)]     = { S(1,7),         E, E},
    [T(TIM8_CH4)]    = { S(7,7),         E, E},
    [T(TIM8_TRIG)]   = { S(7,7),         E, E},
    [T(TIM8_COM)]    = { S(7,7),         E, E}
};

#undef S
#undef E
