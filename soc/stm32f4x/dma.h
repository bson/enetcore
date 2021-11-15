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
        // DMA1
        SPI3_RX = 0, SPI2_RX, SPI2_TX, SPI3_TX, I2C1_RX, TIM7_UP, I2C1_TX,
        TIM4_CH1, I2S3_EXT_RX, TIM4_CH2, I2S2_EXT_TX, TIM4_UP, TIM4_CH3,
        TIM2_UP, TIM2_CH3, I2C3_RX, I2S2_EXT_RX, I2C3_TX, TIM2_CH1, TIM2_CH2, TIM2_CH4,
        UART5_RX, USART3_RX, UART4_RX, USART3_TX, UART4_TX, USART2_RX, USART2_TX,
        UART5_TX, TIM3_CH4, TIM3_UP, TIM3_CH1, TIM3_TRIG, TIM3_CH2, TIM3_CH3, TIM5_CH3,
        TIM5_UP, TIM5_CH4, TIM5_TRIG, TIM5_CH1, TIM5_CH2, TIM6_UP, I2C2_RX, DAC1, DAC2,
        I2C2_TX,

        // DMA2
        ADC1, TIM8_CH1, TIM8_CH2, TIM8_CH3, TIM1_CH1, TIM1_CH2, TIM1_CH3, DCMI, ADC2,
        ADC3, CRYP_OUT, CRYP_IN, HASH_IN, SPI1_RX, SPI1_TX, USART1_RX, SDIO, USART1_TX,
        USART6_RX, USART6_TX, TIM1_TRIG, TIM1_CH4, TIM1_COM, TIM1_UP, TIM8_UP, TIM8_CH4,
        TIM8_TRIG, TIM8_COM,

        NUM_TARGET,
        NOT_USED = NUM_TARGET,
        END = 0xff
    };

    enum {
        NUM_MAPPING = 3
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
        const uint32_t _dr;  // Peripheral TX and RX data register address (common to RX and TX)

        struct Assignment {
            Target  _target;
            uint8_t _stream:3;  // Assigned stream
            uint8_t _ch:3;      // Assigned channel
            volatile bool _active; // Transfer active (used by peripheral)
        };

        Assignment _tx;
        Assignment _rx;

        Priority _prio;         // Priority, common to RX and TX
        WordSize _word_size;    // Word size, common to RX and TX

        uint8_t  _ipl;          // DMA interrupt priority

        Peripheral(uint32_t dr, Target txtarg, Target rxtarg = Target::NOT_USED)
            : _dr(dr),
              _prio(Priority::MEDIUM),
              _word_size(WordSize::BYTE),
              _ipl(IPL_DMA) {
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
    // The device needs to first acquire the DMA stream, via the
    // AssignTx/Rx() calls.  These will block and return with the
    // stream acquired; it is now in the possession of the specified
    // peripheral. Because it blocks it can only be called from a thread
    // context. The stream picked is the first available from the
    // stream-channel pairs for the target in _tx_target, _rx_target.
    //
    // Transmit/Receive can then be issued to perform transfers.  When
    // complete, Peripheral::DmaTxComplete/RxComplete will be called
    // in the DMA interrupt handler context. At this point the device
    // can issue additional Receive/Transmit transfers on the stream.
    // These can be called from an interrupt context, such as the
    // aforementioned DmaTxComplete/RxComplete functions.
    //
    // When finished with the stream, the device must call
    // ReleaseTx/ReleaseTx.  This will wake the next thread waiting to
    // acquire the stream.
    //
    // For Transmit(), if 'minc' is false, don't increment memory
    // address - in other words, copy the word at *buf 'nwords' times.
    // this permits transmitting a single value to a peripheral.  This
    // is mainly useful for large SPI reads where a '0' or 'ff' is
    // clocked out while capturing Rx.
    //
    // TryAssignTx/TryAssignRx are like AssignTx/AssignTx, but
    // won't block. They return true if the stream was acquired,
    // otherwise false.
    //
    // Technically only the Assign methods require the Peripheral*
    // 'p' parameter.  But the others accept it to detect
    // inconsistencies that would produce extremely difficult to track
    // down bugs by asserting the peripheral is holding the stream.
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
