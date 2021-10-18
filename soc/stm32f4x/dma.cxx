#include "enetkit.h"
#include "thread.h"
#include "stm32f4x/dma.h"
#include "nvic.h"

// Interrupt handler
template <uint32_t STREAM, Stm32Dma::Register ISR, Stm32Dma::Register IFCR>
void Stm32Dma::Interrupt(void* token) {
    Stm32Dma* dma = (Stm32Dma*)token;

    static const uint32_t stream_to_tcif[] = {
        BIT(TCIF0), BIT(TCIF1), BIT(TCIF2), BIT(TCIF3),
        BIT(TCIF4), BIT(TCIF5), BIT(TCIF6), BIT(TCIF7)
    };

    const uint32_t tcif = stream_to_tcif[STREAM];
    if (dma->reg(ISR) & tcif) {
        Peripheral* handler = dma->_handler[STREAM];
        dma->reg(IFCR) |= tcif;
        if (handler) {
            if (handler->_tx_active)
                handler->DmaTxComplete();

            handler->_tx_active = false;
            dma->_handler[STREAM] = NULL;
        }
        dma->s_cr(STREAM) &= ~(BIT(TCIE) | BIT(HTIE) | BIT(TEIE) | BIT(DMEIE));
    }
}

void Stm32Dma::InstallHandlers() {
    NVic::InstallIRQHandler(_irq[0], Stm32Dma::Interrupt<0,Register::LISR,Register::LIFCR>, IPL_DMA,
                            this);
    NVic::InstallIRQHandler(_irq[1], Stm32Dma::Interrupt<1,Register::LISR,Register::LIFCR>, IPL_DMA,
                            this);
    NVic::InstallIRQHandler(_irq[2], Stm32Dma::Interrupt<2,Register::LISR,Register::LIFCR>, IPL_DMA,
                            this);
    NVic::InstallIRQHandler(_irq[3], Stm32Dma::Interrupt<3,Register::LISR,Register::LIFCR>, IPL_DMA,
                            this);
    NVic::InstallIRQHandler(_irq[4], Stm32Dma::Interrupt<4,Register::HISR,Register::HIFCR>, IPL_DMA,
                            this);
    NVic::InstallIRQHandler(_irq[5], Stm32Dma::Interrupt<5,Register::HISR,Register::HIFCR>, IPL_DMA,
                            this);
    NVic::InstallIRQHandler(_irq[6], Stm32Dma::Interrupt<6,Register::HISR,Register::HIFCR>, IPL_DMA,
                            this);
    NVic::InstallIRQHandler(_irq[7], Stm32Dma::Interrupt<7,Register::HISR,Register::HIFCR>, IPL_DMA,
                            this);
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
