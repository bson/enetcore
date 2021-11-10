#ifndef __DAC_H__
#define __DAC_H__

// Simplified DAC interface. Uses only channel 1.

class Stm32Dac: public Stm32Dma::Peripheral {
    enum class Register {
        CR = 0x00,
        SWTRIGR = 0x04,
        DHR12R1 = 0x08,
        DHR12L1 = 0x0c,
        DHR8R1 = 0x10,
        DHR12R2 = 0x14,
        DHR12L2 = 0x18,
        DHR8R2 = 0x1c,
        DHR12RD = 0x20,
        DHR12LD = 0x24,
        DHR8RD = 0x28,
        DOR1 = 0x2c,
        DOR2 = 0x30,
        SR = 0x34
    };

    enum {
        // CR
        DMAUDRIE1 = 13,
        DMAEN1 = 12,
        MAMP = 8,
        WAVE1 = 6,
        TSEL1 = 3,
        TEN1 = 2,
        BOFF1 = 1,
        EN1 = 0,

        // SWTRIGR
        SWTRIG2 = 1,
        SWTRIG1 = 0,

        // SR
        DMAUDR2 = 29,
        DMAUDR1 = 13
    };

    uint32_t _base;

    volatile uint32_t& reg(Register r) {
        return *(volatile uint32_t*)(_base + (uint32_t)r);
    }

public:
    Stm32Dac(uint32_t base)
        : Peripheral(base + (uint32_t)Register::DHR12R1),
          _base(base) {
    }

    void Enable() {
        reg(Register::CR) |= BIT(EN1);
    }

    enum class Trigger {
        TIM6_TRGO = 0b000 << TSEL1,
        TIM8_TRGO = 0b001 << TSEL1,
        TIM7_TRGO = 0b010 << TSEL1,
        TIM5_TRGO = 0b011 << TSEL1,
        TIM2_TRGO = 0b100 << TSEL1,
        TIM4_TRGO = 0b101 << TSEL1,
        EXTI9     = 0b110 << TSEL1,
        SWTRIG    = 0b111 << TSEL1
    };

    void Output(const uint16_t* data, uint32_t nsamples, Trigger t,
                Stm32Dma& dma, uint8_t stream, uint8_t dma_ch,
                Stm32Dma::Priority prio) {
        volatile uint32_t& cr = reg(Register::CR);
        cr &= ~BIT(EN1);
        cr = /* BIT(BOFF1)  |*/ BIT(TEN1) | (uint32_t)t | BIT(DMAEN1);
        cr |=  BIT(EN1);

        _tx_size = nsamples;
        _tx_stream = stream;
        _tx_ch = dma_ch;
        _prio = prio;
        _word_size = Stm32Dma::WordSize::WORD16;
        _tx_active = false;
        _ipl = IPL_DAC;
            
        dma.PeripheralTx(this, data, nsamples);
    }

private:
    // Transfer is complete, called in interrupt context
    void DmaTxComplete() { }
    void DmaRxComplete() { }

    // Enable or disable DMA (channel 0)
    void DmaEnableTx() { reg(Register::CR) |= BIT(DMAEN1); }
    void DmaDisableTx() { reg(Register::CR) &= ~BIT(DMAEN1); }
    void DmaEnableRx() { }
    void DmaDisableRx() { }
};

#endif // __DAC_H__
