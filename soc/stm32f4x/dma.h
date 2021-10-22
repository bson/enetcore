#ifndef __DMA_H__
#define __DMA_H__

class Stm32Dma {
public:
    enum class Register {
        LISR = 0x00,
        HISR = 0x04,
        LIFCR = 0x08,
        HIFCR = 0x0c,
        S0CR = 0x10,
        S0NDTR = 0x14,
        S0PAR = 0x18,
        S0M0AR = 0x1c,
        S0M1AR = 0x20,
        S0FCR = 0x24,
    };

    enum {
        // LISR
        TCIF3 = 27,
        HTIF3 = 26,
        TEIF3 = 25,
        DMEIF3 = 24,
        FFEIF3 = 22,
        TCIF2 = 21,
        HTIF2 = 20,
        TEIF2 = 19,
        DMEIF2 = 18,
        FEIF2 = 16,
        TCIF1 = 11,
        HTIF1 = 10,
        TEIF1 = 9,
        DMEIF1 = 8,
        FEIF1 = 6,
        TCIF0 = 5,
        HTIF0 = 4,
        TEIF0 = 3,
        DMEIF0 = 2,
        FEIF0 = 0,

        // HISR
        TCIF7 = 27,
        HTIF7 = 26,
        TEIF7 = 25,
        DMEIF7 = 24,
        FFEIF7 = 22,
        TCIF6 = 21,
        HTIF6 = 20,
        TEIF6 = 19,
        DMEIF6 = 18,
        FEIF6 = 16,
        TCIF5 = 11,
        HTIF5 = 10,
        TEIF5 = 9,
        DMEIF5 = 8,
        FEIF5 = 6,
        TCIF4 = 5,
        HTIF4 = 4,
        TEIF4 = 3,
        DMEIF4 = 2,
        FEIF4 = 0,

        // LIFCR
        CTCIF3 = 27,
        CHTIF3 = 26,
        CTEIF3 = 25,
        CDMEIF3 = 24,
        CFFEIF3 = 22,
        CTCIF2 = 21,
        CHTIF2 = 20,
        CTEIF2 = 19,
        CDMEIF2 = 18,
        CFEIF2 = 16,
        CTCIF1 = 11,
        CHTIF1 = 10,
        CTEIF1 = 9,
        CDMEIF1 = 8,
        CFEIF1 = 6,
        CTCIF0 = 5,
        CHTIF0 = 4,
        CTEIF0 = 3,
        CDMEIF0 = 2,
        CFEIF0 = 0,

        // HIFCR
        CTCIF7 = 27,
        CHTIF7 = 26,
        CTEIF7 = 25,
        CDMEIF7 = 24,
        CFFEIF7 = 22,
        CTCIF6 = 21,
        CHTIF6 = 20,
        CTEIF6 = 19,
        CDMEIF6 = 18,
        CFEIF6 = 16,
        CTCIF5 = 11,
        CHTIF5 = 10,
        CTEIF5 = 9,
        CDMEIF5 = 8,
        CFEIF5 = 6,
        CTCIF4 = 5,
        CHTIF4 = 4,
        CTEIF4 = 3,
        CDMEIF4 = 2,
        CFEIF4 = 0,

        // SxCR
        CHSEL = 25,
        MBURST = 23,
        PBURST = 21,
        CT = 19,
        DBM = 18,
        PL = 16,
        PINCOS = 15,
        MSIZE = 13,
        PSIZE = 11,
        MINC = 10,
        PINC = 9,
        CIRC = 8,
        DIR = 6,
        PFCTRL = 5,
        TCIE = 4,
        HTIE = 3,
        TEIE = 2,
        DMEIE = 1,
        EN = 0,

        // SxFCR
        FEIE = 7,
        FS = 3,
        DMDIS = 2,
        FTH = 0,
    };

    [[__finline, __optimize]]
    volatile uint32_t& reg(Register r) {
        return *(volatile uint32_t*)(_base + (uint32_t)r);
    }

    [[__finline, __optimize]]
    volatile uint32_t& stream_reg(Register r, uint32_t stream) {
        return *(volatile uint32_t*)(_base + (uint32_t)r + stream*0x18);
    }

    // Stream registers
    [[__finline, __optimize]]
    volatile uint32_t& s_cr(uint32_t stream) { return stream_reg(Register::S0CR, stream); }

    [[__finline, __optimize]]
    volatile uint32_t& s_ndtr(uint32_t stream) { return stream_reg(Register::S0NDTR, stream); }

    [[__finline, __optimize]]
    volatile uint32_t& s_par(uint32_t stream) { return stream_reg(Register::S0PAR, stream); }

    [[__finline, __optimize]]
    volatile uint32_t& s_m0ar(uint32_t stream) { return stream_reg(Register::S0M0AR, stream); }

    [[__finline, __optimize]]
    volatile uint32_t& s_m1ar(uint32_t stream) { return stream_reg(Register::S0M1AR, stream); }

    [[__finline, __optimize]]
    volatile uint32_t& s_fcr(uint32_t stream) { return stream_reg(Register::S0FCR, stream); }

public:
    enum class Priority {
        LOW = 0,
        MEDIUM = 1,
        HIGH = 2,
        VERY_HIGH = 3
    };

    enum class WordSize {
        BYTE   = 0,
        WORD16 = 1,
        WORD32 = 2
    };

    class Peripheral {
    public:
        const uint32_t _tx_dr;        // Peripheral data register address

        uint16_t _tx_size;      // Size of current or most recent transfer
        uint8_t  _tx_ch;
        uint8_t  _tx_stream;
        Priority _tx_prio;
        WordSize _tx_word_size;
        bool     _tx_active;
        uint8_t  _ipl;

        Peripheral(uint32_t txdr)
            : _tx_dr(txdr),
              _tx_prio(Priority::MEDIUM),
              _tx_word_size(WordSize::BYTE),
              _tx_active(false),
              _ipl(IPL_DMA) {
        }

        // Transfer is complete, called in interrupt context
        virtual void DmaTxComplete() = 0;

        // Enable or disable DMA
        virtual void DmaEnable() = 0;
        virtual void DmaDisable() = 0;
    };

private:
    const uint32_t  _base;
    Peripheral*     _handler[8];
    const uint16_t* _irq;

    static const uint32_t stream_to_tcif[8];

public:
    Stm32Dma(uint32_t base, const uint16_t* irqs)
        : _base(base), _irq(irqs) {
    };

    // Transfer to peripheral
    void PeripheralTx(Stm32Dma::Peripheral* p, const void* buf, uint16_t nwords) {
        assert(nwords <= 0xffff);
        assert(nwords != 0);
        assert(buf != NULL);
        assert(p);

        volatile uint32_t& cr = s_cr(p->_tx_stream);

        NVic::SetIRQPriority(_irq[p->_tx_stream], p->_ipl);

        Thread::IPL G(p->_ipl);

        cr &= ~BIT(EN);
        ClearTCIF(p->_tx_stream);

        cr = (p->_tx_ch << CHSEL)
            | ((uint32_t)p->_tx_prio << PL)
            | ((uint32_t)p->_tx_word_size << MSIZE)
            | ((uint32_t)p->_tx_word_size << PSIZE)
            | (1 << DIR)
            | BIT(MINC) | BIT(TCIE);

        s_fcr(p->_tx_stream) = BIT(DMDIS);
        s_par(p->_tx_stream) = p->_tx_dr;
        s_m0ar(p->_tx_stream) = (uint32_t)buf;
        s_ndtr(p->_tx_stream) = nwords;

        _handler[p->_tx_stream] = p;

        // Make it so
        p->DmaEnable();
        p->_tx_active = true;
        cr |= BIT(EN);
    }

    // Clear TCIF for stream
    void ClearTCIF(uint32_t stream);

	// Interrupt handler
    template <uint32_t STREAM, Register ISR, Register IFCR>
	static void Interrupt(void* token);

    // Install NVIC interrupt handlers
    void InstallHandlers();

    // Enable NVIC interrupts
    void EnableInterrupts();

private:
    Stm32Dma(const Stm32Dma&);
    Stm32Dma& operator=(const Stm32Dma&);
};

#endif // __DMA_H__
