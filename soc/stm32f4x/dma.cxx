#include "core/enetkit.h"
#include "core/thread.h"
#include "soc/stm32f4x/dma.h"
#include "arch/armv7m/nvic.h"


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
            handler->_tx_active = false;
            handler->DmaDisable();       // Remove DMA trigger
            handler->DmaTxComplete();
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
    NVic::InstallIRQHandler(_irq[0], Stm32Dma::Interrupt<0,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::InstallIRQHandler(_irq[1], Stm32Dma::Interrupt<1,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::InstallIRQHandler(_irq[2], Stm32Dma::Interrupt<2,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::InstallIRQHandler(_irq[3], Stm32Dma::Interrupt<3,Register::LISR,Register::LIFCR>, IPL_DMA, this);
    NVic::InstallIRQHandler(_irq[4], Stm32Dma::Interrupt<4,Register::HISR,Register::HIFCR>, IPL_DMA, this);
    NVic::InstallIRQHandler(_irq[5], Stm32Dma::Interrupt<5,Register::HISR,Register::HIFCR>, IPL_DMA, this);
    NVic::InstallIRQHandler(_irq[6], Stm32Dma::Interrupt<6,Register::HISR,Register::HIFCR>, IPL_DMA, this);
    NVic::InstallIRQHandler(_irq[7], Stm32Dma::Interrupt<7,Register::HISR,Register::HIFCR>, IPL_DMA, this);
}

void Stm32Dma::EnableInterrupts() {
    NVic::EnableIRQ(_irq[0]);
    NVic::EnableIRQ(_irq[1]);
    NVic::EnableIRQ(_irq[2]);
    NVic::EnableIRQ(_irq[3]);
    NVic::EnableIRQ(_irq[4]);
    NVic::EnableIRQ(_irq[5]);
    NVic::EnableIRQ(_irq[6]);
    NVic::EnableIRQ(_irq[7]);
}
