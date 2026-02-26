// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

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
        SR = 0x34,
        CCR = 0x38,
        MCR = 0x3c,
        SHSR1 = 0x40,
        SHSR2 = 0x44,
        SHHR = 0x48,
        SHRR = 0x4c
    };

    enum {
        // CR
        CEN2 = 30,
        DMAUDRIE2 = 29,
        DMAEN2 = 28,
        MAMP2 = 24,
        WAVE2 = 22,
        TSEL2 = 18,
        TEN2 = 17,
        EN2 = 16,
        CEN1 = 14,
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
        BWST2 = 31,
        CAL_FLAG2 = 30,
        DMAUDR2 = 29,
        BWST1 = 15,
        CAL_FLAG1 = 14,
        DMAUDR1 = 13,

        // CCR
        OTRIM2 = 16,
        OTRIM1 = 0,

        // MCR
        MODE2 = 16,
        MODE1 = 0,

        // SHHR
        THOLD2 = 16,
        THOLD1 = 0,

        // SHRR
        TREFRESH2 = 16,
        TREFRESH1 = 0,
    };

    uintptr_t _base;

    volatile uint32_t& reg(Register r) {
        return *(volatile uint32_t*)(_base + (uint32_t)r);
    }

public:
    Stm32Dac(uint32_t base, Stm32Dma::Target tx_targ, uint32_t streams)
        : Peripheral(base + (uint32_t)Register::DHR12R1, 0,
                     tx_targ, Stm32Dma::Target::NOT_USED, streams),
          _base(base) {
    }

    void Enable() {
        reg(Register::CR) |= BIT(EN1);
    }

    enum class Trigger {
        SWTRIG = 0,
        TIM1   = 1,
        TIM2   = 2,
        TIM4   = 3,
        TIM5   = 4,
        TIM6   = 5,
        TIM7   = 6,
        TIM8   = 7,
        TIM15  = 8,
        HRTIM1_DACTRG1 = 9,
        HRTIM1_DACTRG2 = 10,
        LPTIM1 = 11,
        LPTIM2 = 12,
        EXTI9  = 13,
    };

    void Output(const uint16_t* data, uint32_t nsamples, Trigger t,
                Stm32Dma& dma, uint8_t stream, uint8_t dma_ch,
                Stm32Dma::Priority prio) {
        volatile uint32_t& cr = reg(Register::CR);
        cr &= ~BIT(EN1);
        cr = Bitfield(cr)
            .bit(BOFF1)
            .bit(TEN1)
            .f(4, TSEL1, uint32_t(t))
            .bit(DMAEN1);
        cr |= BIT(EN1);

        _prio = prio;
        _word_size = Stm32Dma::WordSize::WORD16;
        _ipl = IPL_DAC;
            
        dma.AssignTx(this);
        dma.Transmit(this, data, nsamples, true);
        dma.ReleaseTx(this);
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
