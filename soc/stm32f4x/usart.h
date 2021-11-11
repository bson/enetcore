// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_USART_H__
#define __STM32_USART_H__

#include "core/ring.h"
#include "core/mutex.h"
#include "core/consumer.h"

template <uint32_t SEND_BUF_SIZE = 128, uint32_t RECV_BUF_SIZE = 32>
class Stm32Usart: public Stm32Dma::Peripheral,
                  public Consumer<uint8_t> {
public:
    enum class Register {
        USART_SR   = 0x00,
        USART_DR   = 0x04,
        USART_BRR  = 0x08,
        USART_CR1  = 0x0c,
        USART_CR2  = 0x10,
        USART_CR3  = 0x14,
        USART_GTPR = 0x18
    };

    enum {
        // USART_SR
        CTS   = 9,
        LBD   = 8,
        TXE   = 7,
        TC    = 6,
        RXNE  = 5,
        IDLE  = 4,
        ORE   = 3,
        NF    = 2,
        FE    = 1,
        PE    = 0,

        // USART_DR is a simple 8 bit data register

        // USART_BRR - Q12.4 fixed-point value
        DIV_MANT = 4,
        DIV_FRAC = 0,

        // USART_CR1
        OVER8  = 15,
        UE     = 13,
        M      = 12,
        WAKE   = 11,
        PCE    = 10,
        PS     = 9,
        PEIE   = 8,
        TXEIE  = 7,
        TCIE   = 6,
        RXNEIE = 5,
        IDLEIE = 4,
        TE     = 3,
        RE     = 2,
        RWU    = 1,
        SBK    = 0,

        // USART_CR2
        LINEN = 14,
        STOP  = 12,
        CLKEN = 11,
        CPOL  = 10,
        CPHA  = 9,
        LBCL  = 8,
        LBDIE = 6,
        LBDL  = 5,
        ADD   = 0,

        // USART_CR3
        ONEBIT = 11,
        CTSIE  = 10,
        CTSE   = 9,
        RTSE   = 8,
        DMAT   = 7,
        DMAR   = 6,
        SCEN   = 5,
        NACK   = 4,
        HDSEL  = 3,
        IRLP   = 2,
        IREN   = 1,
        EIE    = 0,

        // USART_GTPR
        GT  = 8,
        PSC = 0
    };

    enum StopBits {
        SB_1   = 0,            // 1
        SB_0_5 = 1,            // 0.5 (smartcard mode only)
        SB_2   = 2,            // 2
        SB_1_5 = 3             // 1.5
    };

private:
    typedef Ring<SEND_BUF_SIZE> SendQ;
    typedef Ring<RECV_BUF_SIZE> RecvQ;

    const uintptr_t _base;
    SendQ _sendq;
    RecvQ _recvq;
    mutable Mutex _w_mutex;
    uint8_t _read_wait;         // Readers waiting
    bool _ienable;              // Enable interrupts

    // For DMA
    Stm32Dma* _dma;          // TX DMA is enabled if this != NULL
    uint16_t _tx_size;       // Current DMA TX length

public:
    Stm32Usart(const uintptr_t base)
        : Peripheral(base + (uint32_t)Register::USART_DR),
          _base(base),
          _ienable(false),
          _read_wait(0),
          _dma(NULL) {
    }

    template <typename T>
    T& reg(const Register r) {
        return *((T*)(_base + (uint32_t)r)); 
    }

    // Set up for async use.  8 bits no parity.  1 or 2 stop bits.
    void InitAsync(uint32_t baudrate, StopBits stopbit, uint32_t timerclk) {
        // BRR is Q12.4, so
        // BRR*16 = clk/(16*baud)   => BRR = clk/baud
        // XXX Simple truncatation, maybe use Q12.5 and round manually, but probably good enough (<< 1% error)
        const uint32_t brr = timerclk/baudrate;
        assert(brr <= 0xffff);

        Thread::IPL G(IPL_UART);

        volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
        cr1 &= ~(BIT(PEIE) | BIT(TXEIE) | BIT(TCIE) | BIT(RXNEIE) | BIT(TE) | BIT(RE)
                 | BIT(SBK) | BIT(RWU) | BIT(M) | BIT(UE) | BIT(PCE) | BIT(IDLEIE)
                 | BIT(OVER8));

        volatile uint32_t& cr2 = reg<volatile uint32_t>(Register::USART_CR2);
        cr2 &= ~(BIT(LINEN) | (3 << STOP) | BIT(CLKEN) | BIT(CPOL) | BIT(CPHA) | BIT(LBCL)
                 | BIT(LBDIE) | 0xf);

        volatile uint32_t& cr3 = reg<volatile uint32_t>(Register::USART_CR3);
        cr3 = (cr3 & ~(BIT(CTSIE) | BIT(CTSE) | BIT(RTSE) | BIT(DMAT) | BIT(DMAR) | BIT(SCEN)
                       | BIT(NACK) | BIT(HDSEL) | BIT(IRLP) | BIT(IREN) | BIT(EIE)))
            | BIT(ONEBIT);
    
        reg<volatile uint32_t>(Register::USART_BRR) = brr;

        cr2 |= (uint32_t)stopbit << STOP;
    
        cr1 &= ~BIT(OVER8);
        cr1 |= BIT(UE);
        cr1 |= BIT(TE) | BIT(RE);
    }
    
    // Write buffer
    void Write(const uint8_t* data, uint len) {
        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_UART);
    
        for (const uint8_t *p = data; p < data + len; ) {
            if (_sendq.Headroom()) {
                while (_sendq.Headroom() && (p < data + len))
                    _sendq.PushBack(*p++);

                if (_dma)
                    _dma->AcquireTx(this);

                StartTx();
            }
            if (p < data + len)
                Thread::WaitFor((void*)&_sendq);
        }
    }


	// Write string
	void Write(const String& s) { Write(s.CStr(), s.Size()); }

    // Write C string
    void WriteCStr(const char* s) { Write((const uint8_t*)s, strlen(s)); }

    // Consumer interface
    void Apply(uint8_t c) { Write(&c, 1); }
    void Apply(const uint8_t* buf, uint len) { Write(buf, len); }
    void Apply(const String& s) { Write(s); }

    // Enable DMA TX
    void EnableDmaTx(Stm32Dma& dma, uint8_t stream, uint8_t ch, Stm32Dma::Priority prio) {
        assert(stream <= 7);
        assert(ch <= 7);

        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_UART);

        _ipl = IPL_UART;
        _dma = &dma;
        _tx_stream = stream;
        _tx_ch = ch;
        _prio = prio;
        _word_size = Stm32Dma::WordSize::BYTE;
        _tx_active = false;

        SetInterrupts(_ienable);
    }

    // getc
    int getc() {
        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_UART);

        for (;;) {
            if (!_recvq.Empty())
                return _recvq.PopFront();

            ++_read_wait;
            Thread::WaitFor((void*)&_recvq);
            --_read_wait;
        }
    }

	// Drain write buffer synchronously (= polled)
    void SyncDrain() {
        ScopedNoInt G;

        while (!_sendq.Empty())
            StartTx();
    }

	// Enable interrupts
    void SetInterrupts(bool enable) {
        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_UART);

        _ienable = enable;

        volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
        if (enable) {
            cr1 = (cr1 & ~BIT(TXEIE)) | (_dma ? 0 : BIT(TXEIE)) | BIT(RXNEIE);
        } else {
            cr1 &= ~(BIT(TXEIE) | BIT(RXNEIE));
        }
    }

	// Interrupt handler
	static void Interrupt(void* token) {
        ((Stm32Usart*)token)->HandleInterrupt();
    }

    // DMA TX complete
    void DmaTxComplete() {
        _sendq.PopFront(exch(_tx_size, uint16_t(0)));
        StartTx();
        Thread::WakeSingle((void*)&_sendq);
    }

    // NYI
    void DmaRxComplete() { }

    // Enable disable DMA (attach, detach trigger)
    void DmaEnableTx() {
        reg<volatile uint32_t>(Register::USART_CR3) |= BIT(DMAT);
    }

    void DmaDisableTx() {
        reg<volatile uint32_t>(Register::USART_CR3) &= ~BIT(DMAT);
    }

    void DmaEnableRx() { }
    void DmaDisableRx() { }
private:

    inline void StartTx() {
        if (_dma) {
            if (!_sendq.Empty()) {
                if (!_tx_active)
                    _dma->Transmit(this, _sendq.Buffer(), (_tx_size = _sendq.Continuous()),
                                   true);
            } else {
                _sendq.Clear();     // Normalize
                _dma->ReleaseTx(this);
            }

            // Just in case we got here on a TXE interrupt, right after enabling DMA
            reg<volatile uint32_t>(Register::USART_CR1) &= ~BIT(TXEIE);
            return;
        }

        // Not DMA - use interrupt based Tx

        // The TXE interrupt is a bit... unusual.  When TXE is high it interrupts continuously
        // and if we don't have anything to send the only way to make it stop is to disable
        // interrupts.  This makes for some rather messy code.
        volatile uint32_t& sr = reg<volatile uint32_t>(Register::USART_SR);
        if (sr & BIT(TXE)) {
            volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
            if (!_sendq.Empty()) {
                volatile uint32_t& dr = reg<volatile uint32_t>(Register::USART_DR);
                dr = _sendq.PopFront();
                if (_ienable)
                    cr1 |= BIT(TXEIE);
            } else {
                cr1 &= ~BIT(TXEIE);
            }
        }
    }

    inline void HandleInterrupt() {
        volatile uint32_t& sr = reg<volatile uint32_t>(Register::USART_SR);
    
        if (sr & BIT(TXE)) {
            StartTx();
            if (_sendq.Empty())
                Thread::WakeSingle((void*)&_sendq);
        }

        if (sr & BIT(RXNE)) {
            volatile uint32_t& dr = reg<volatile uint32_t>(Register::USART_DR);
            const uint8_t c = (uint8_t)dr;
            if (_recvq.Headroom()) {
                _recvq.PushBack(c);
                if (_read_wait)
                    Thread::WakeSingle((void*)&_recvq);
            }
        }
    }


    Stm32Usart(const Stm32Usart&);
    Stm32Usart& operator=(const Stm32Usart&);
};

#endif // __STM32_USART_H__
