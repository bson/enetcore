#ifndef __DMA_H__
#define __DMA_H__

class Stm32Dma {
public:
    enum class Register {
        // DMA
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

        // BASE_DMAMUX1; DMA1, DMA2 (D2 domain)
        // BASE_DMAMUX2: BDMA (D3 domain)
        DMAMUX_C1CR = 0x000,   // 0x000 + 0x04 * x (x = 0..15)
        DMAMUX_CSR = 0x80,
        DMAMUX_CFR = 0x84,
        DMAMUX_RG1CR = 0x100,  // 0x100 + 0x04 * x (x = 0..7)
        DMAMUX_RGSR = 0x140,
        DMAMUX_RGCFR = 0x144,
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
        MBURST = 23,
        PBURST = 21,
        TRBUFF = 20,            // Enable bufferable transfers (set for UART)  XXXX
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

        // DMAMUX_CxCR
        SYNC_ID = 24,
        NBREQ = 19,
        SPOL = 17,
        SE = 16,
        EGE = 9,
        SOIE = 8,
        DMAREQ_ID = 0,


        // DMAMUX_RGxCR
        GNBREQ = 19,
        GPOL = 17,
        GE = 16,
        OIE = 9,
        SIG_ID = 0,

    };

    enum Direction: uint32_t {
        PTOM = 0b00 << DIR,
        MTOP = 0b01 << DIR,
        MTOM = 0b10 << DIR
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
    enum class Target: uint8_t {
        // Note: the value is the DMA request MUX input
        enum {
            reserved_0 = 0, dmamux1_req_gen0 = 1, dmamux1_req_gen1 = 2, dmamux1_req_gen2 = 3,
            dmamux1_req_gen3 = 4, dmamux1_req_gen4 = 5, dmamux1_req_gen5 = 6, dmamux1_req_gen6 = 7,
            dmamux1_req_gen7 = 8, adc1_dma = 9, adc2_dma = 10, TIM1_CH1 = 11, TIM1_CH2 = 12,
            TIM1_CH3 = 13, TIM1_CH4 = 14, TIM1_UP = 15, TIM1_TRIG = 16, TIM1_COM = 17, TIM2_CH1 = 18,
            TIM2_CH2 = 19, TIM2_CH3 = 20, TIM2_CH4 = 21, TIM2_UP = 22, TIM3_CH1 = 23, TIM3_CH2 = 24,
            TIM3_CH3 = 25, TIM3_CH4 = 26, TIM3_UP = 27, TIM3_TRIG = 28, TIM4_CH1 = 29, TIM4_CH2 = 30, 
            TIM4_CH3 = 31, TIM4_UP = 32, i2c1_rx_dma = 33, i2c1_tx_dma = 34, i2c2_rx_dma = 35,
            i2c2_tx_dma = 36, spi1_rx_dma = 37, spi1_tx_dma = 38, spi2_rx_dma = 39, spi2_tx_dma = 40, 
            usart1_rx_dma = 41, usart1_tx_dma = 42, usart2_rx_dma = 43, usart2_tx_dma = 44,
            usart3_rx_dma = 45, usart3_tx_dma = 46, TIM8_CH1 = 47, TIM8_CH2 = 48, TIM8_CH3 = 49, 
            TIM8_CH4 = 50, TIM8_UP = 51, TIM8_TRIG = 52, TIM8_COM = 53, Reserved_54 = 54, TIM5_CH1 = 55,
            TIM5_CH2 = 56, TIM5_CH3 = 57, TIM5_CH4 = 58, TIM5_UP = 59, TIM5_TRIG = 60, spi3_rx_dma = 61,
            spi3_tx_dma = 62, uart4_rx_dma = 63, uart4_tx_dma = 64, uart5_rx_dma = 65, 
            uart5_tx_dma = 66, dac_ch1_dma = 67, dac_ch2_dma = 68, TIM6_UP = 69, TIM7_UP = 70,
            usart6_rx_dma = 71, usart6_tx_dma = 72, i2c3_rx_dma = 73, i2c3_tx_dma = 74, dcmi_dma = 75,
            cryp_in_dma = 76, cryp_out_dma = 77, hash_in_dma = 78, uart7_rx_dma = 79, uart7_tx_dma = 80,
            uart8_rx_dma = 81, uart8_tx_dma = 82, spi4_rx_dma = 83, spi4_tx_dma = 84, spi5_rx_dma = 85,
            spi5_tx_dma = 86, sai1a_dma = 87, sai1b_dma = 88, sai2a_dma = 89, sai2b_dma = 90, 
            swpmi_rx_dma = 91, swpmi_tx_dma = 92, spdifrx_dat_dma = 93, spdifrx_ctrl_dma = 94,
            HR_REQ_ = 95, HR_REQ_2 = 96, HR_REQ_3 = 97, HR_REQ_4 = 98, HR_REQ_5 = 99, HR_REQ_6 = 100,
            dfsdm1_dma0 = 101, dfsdm1_dma1 = 102, dfsdm1_dma2 = 103, dfsdm1_dma3 = 104, TIM15_CH1 = 105,
            TIM15_UP = 106, TIM15_TRIG = 107, TIM15_COM = 108, TIM16_CH1 = 109, TIM16_UP = 110,
            TIM17_CH1 = 111, TIM17_UP = 112, sai3_a_dma = 113, sai3_b_dma = 114, adc3_dma = 115
            NUM_TARGET,
            NOT_USED = NUM_TARGET,
        };
    };

    static_assert(Target::NUM_TARGET < Target::END);

    enum {
        NUM_MAPPING = 3         // Max number of stream-channel options for any target
    };

    enum class Priority: uint8_t {
        LOW = 0,
        MEDIUM = 1,
        HIGH = 2,
        VERY_HIGH = 3
    };

    enum class WordSize: uint8_t {
        BYTE   = 0,
        WORD16 = 1,
        WORD32 = 2
    };

    class Peripheral {
    public:
        struct Assignment {
            const uintptr_t _dr;   // Data register
            uint16_t _n_ops;       // # of DMA operations per request
            Target  _target;       // Peripheral ID
            uint8_t _stream:3;     // Assigned stream
            volatile bool _active; // Transfer active
        };

        Assignment _tx;
        Assignment _rx;

        Priority _prio;         // Priority, common to RX and TX
        WordSize _word_size;    // Word size, common to RX and TX

        uint8_t  _ipl;          // DMA interrupt priority
        bool     _trbuff;       // Buffered transfer

        Peripheral(uint32_t tdr, uin32_t rdr, Target txtarg, Target rxtarg = Target::NOT_USED,
                   uint16_t rdop = 1, uint16_t wrop = 1)
            : _prio(Priority::MEDIUM),
              _word_size(WordSize::BYTE),
              _ipl(IPL_DMA),
              _trbuff(false) {
            _tx._dr     = tdr;
            _rx._dr     = rdr;
            _tx._n_ops  = wrop;
            _rx._n_ops  = rdop;
            _tx._target = txtarg;
            _rx._target = rxtarg;
            _tx._active = false;
            _rx._active = false;
        }

        // Transfer is complete, called in interrupt context
        virtual void DmaTxComplete() = 0;
        virtual void DmaRxComplete() = 0;

        // Enable or disable DMA
        virtual void DmaEnableTx() = 0;
        virtual void DmaDisableTx() = 0;

        virtual void DmaEnableRx() = 0;
        virtual void DmaDisableRx() = 0;
    };

private:
    const uint32_t  _base;
    const uint16_t* _irq;
    Peripheral*     _assignment[8];
    bool            _is_tx[8];  // Tracks whether stream is active for TX or RX

    static const uint32_t stream_to_tcif[8];

    // Target to stream-channel table.  Each stream-channel pair is
    // encoded in a byte with the stream in the high nybble and the
    // channel in the low.  Each list is ended with a 0xff sentry.
    static const uint8_t _target_sch[(int)Target::NUM_TARGET][NUM_MAPPING];

public:
    Stm32Dma(uint32_t base, const uint16_t* irqs)
        : _base(base), _irq(irqs) {
        memset(_assignment, 0, sizeof _assignment);
        memset(_is_tx, 0, sizeof _is_tx);
    };

    // Peripheral transfers
    //
    // XXX this needs updating for H7x since there is now a DMAMUX for
    // request to channel mapping.
    //
    // The device needs to first acquire a DMA stream-channel pair,
    // via the AssignTx/Rx() calls.  These will block and return with
    // a stream acquired when one becomes available; the device is now
    // in possession of the selected stream+channel. Because these
    // calls block they can only be called from a thread context. The
    // stream picked is the first available from the stream-channel
    // pairs for the target in the Peripheral's _tx._target and
    // _rx._target.
    //
    // TBD: Optionally a device can declare a preferred s-ch selection
    // order by setting it in the Peripheral.  This can be used to
    // control which DMA resource(s) each device will contend over.
    //
    // TBD: Exclusive access can be obtained by specifying a single
    // s-ch, then acquiring it up front and never releasing.  This
    // will prevent any other peripheral from using the same s-ch.
    //
    // Once acquired, Transmit/Receive functions are used to perform
    // transfers.  When complete, the Peripheral::DmaTxComplete and
    // DmaRxComplete callbacks will be called in the DMA interrupt
    // handler context. At this point the device can issue additional
    // Receive/Transmit transfers on the stream.  These can be called
    // from an interrupt context, such as the aforementioned
    // DmaTxComplete/RxComplete functions to reduce thread context
    // switch overhead.
    //
    // When finished with the stream, the device must call
    // ReleaseTx/ReleaseTx.  This will wake the next thread waiting to
    // acquire the stream.  These can be called in an interrupt
    // context, such as in DmaTxComplete or DmaRxComplete.
    //
    // For Transmit(), if 'minc' is false, don't increment the memory
    // address - in other words, copy the word at *buf 'nwords' times.
    // this permits transmitting a single value to a peripheral.  This
    // is mainly useful for large SPI reads where a '0' or 'ff' is
    // clocked out while capturing Rx.  This way the DMA can keep
    // feeding a single repeating byte or word on the MOSI pin.
    //
    // TryAssignTx/TryAssignRx are like AssignTx/AssignTx, but
    // won't block. They return true if the stream was acquired,
    // otherwise false.
    //
    // Technically only the Assign methods require the Peripheral* 'p'
    // parameter.  But the others accept it to detect inconsistencies
    // that would produce extremely difficult to track down bugs by
    // asserting the peripheral is holding the stream.  In a fat
    // single-file production build they're optimized out of
    // existence.
    //
    // TBD a function NYI can be used to perform memory-to-memory
    // copies.
    //
    void AssignTx(Peripheral* p);
    bool TryAssignTx(Peripheral* p);
    void Transmit(Peripheral* p, const void* buf, uint16_t nwords, bool minc);
    void ReleaseTx(Peripheral* p);

    void AssignRx(Peripheral* p);
    bool TryAssignRx(Peripheral* p);
    void Receive(Peripheral* p, void* buf, uint16_t nwords);
    void ReleaseRx(Peripheral* p);

    // Assign both RX & TX.  Avoids deadlocking.
    void AssignRxTx(Stm32Dma::Peripheral* p);

    // Underlying implementation
    bool TryAssign(Peripheral* p, Peripheral::Assignment& asn, bool istx);

    // Clear TCIF for stream
    void ClearTCIF(uint32_t stream);

	// Interrupt handler
    template <uint32_t STREAM, Register ISR, Register IFCR>
	static void Interrupt(void* token);

    // Install NVIC interrupt handlers
    void InstallHandlers();

private:
    Stm32Dma(const Stm32Dma&);
    Stm32Dma& operator=(const Stm32Dma&);
};

#endif // __DMA_H__
