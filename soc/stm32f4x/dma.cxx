#include "enetkit.h"
#include "thread.h"
#include "stm32f4x/dma.h"

void Stm32Dma::HandleInterrupt() {
    static const uint32_t stream_to_tcif[] = {
        BIT(TCIF0), BIT(TCIF1), BIT(TCIF2), BIT(TCIF3),
        BIT(TCIF4), BIT(TCIF5), BIT(TCIF6), BIT(TCIF7)
    };

    uint32_t lisr = reg(Register::LISR);
    if (lisr) {
        for (uint stream = 0; lisr && stream <= 3; stream++) {
            const uint32_t bit = stream_to_tcif[stream];
            if (lisr & bit) {
                reg(Register::LIFCR) |= bit;
                if (_handler[stream])
                    _handler[stream]->DmaComplete();
                _handler[stream] = NULL;
                s_cr(stream) &= ~(BIT(TCIE) | BIT(HTIE) | BIT(TEIE) | BIT(DMEIE));
                lisr &= ~bit;
            }
        }
        reg(Register::LIFCR) = 0b00001111011111010000111101111101;
    }

    uint32_t hisr = reg(Register::HISR);
    if (hisr) {
        for (uint stream = 4; hisr && stream <= 7; stream++) {
            const uint32_t bit = stream_to_tcif[stream];
            if (hisr & bit) {
                reg(Register::HIFCR) |= bit;
                if (_handler[stream])
                    _handler[stream]->DmaComplete();
                _handler[stream] = NULL;
                s_cr(stream) &= ~(BIT(TCIE) | BIT(HTIE) | BIT(TEIE) | BIT(DMEIE));
                hisr &= ~bit;
            }
        }
        reg(Register::HIFCR) = 0b00001111011111010000111101111101;
    }
}


// Interrupt handler
void Stm32Dma::Interrupt(void* token) {
    ((Stm32Dma*)token)->HandleInterrupt();
}
