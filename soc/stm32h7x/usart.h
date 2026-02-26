// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_USART_H__
#define __STM32_USART_H__

#include "core/time.h"
#include "core/ring.h"
#include "core/mutex.h"
#include "core/consumer.h"
#include "core/bitfield.h"


template <uint32_t SEND_BUF_SIZE = 128, uint32_t RECV_BUF_SIZE = 32>
class Stm32Usart: public Stm32Dma::Peripheral,
                  public Consumer<uint8_t> {
public:
    enum class Register {
        USART_CR1  = 0x00,
        USART_CR2  = 0x04,
        USART_CR3  = 0x08,
        USART_BRR  = 0x0c,
        USART_GTPR = 0x10,
        USART_RTOR = 0x14,
        USART_RQR  = 0x18,
        USART_ISR  = 0x1c,
        USART_ICR  = 0x20,
        USART_RDR  = 0x24,
        USART_TDR  = 0x28,
        USART_PRESC = 0x2c
    };

    enum {
        // USART_CR1
        RXFFIE = 31,
        TXFEIE = 30,
        FIFOEN = 29,
        M1 = 28,
        EOBIE = 27,
        RTOIE = 26,
        DEAT = 21,
        DEDT = 16,
        OVER8 = 15,
        CMIE = 14,
        MME = 13,
        M0 = 12,
        WAKE = 11,
        PCE = 10,
        PS = 9,
        PEIE = 8,
        TXFNFIE = 7,
        TCIE = 6,
        RXFNEIE = 5,
        IDLEIE = 4,
        TE = 3,
        RE = 2,
        UESM = 1,
        UE = 0,

        // USART_CR2
        ADD = 24,
        RTOEN = 23,
        ABRMOD = 21,
        ABREN = 20,
        MSBFIRST = 19,
        DATAINV = 18,
        TXINV = 17,
        RXINV = 16,
        SWAP = 15,
        LINEN = 14,
        STOP = 12,
        CLKEN = 11,
        CPOL = 10,
        CPHA = 9,
        LBCL = 8,
        LBDIE = 6,
        LBDL = 5,
        ADDM7 = 4,
        DIS_NSS = 3,
        SLVEN = 0,

        // USART_CR3
        TXFTCFG = 29,
        RXFTIE = 28,
        RXFTCFG = 25,
        TCBGTIE = 24,
        TXFTIE = 23,
        WUFIE = 22,
        WUS = 20,
        SCARCNT = 17,
        DEP = 15,
        DEM = 15,
        DDRE = 13,
        OVRDIS = 12,
        ONEBIT = 11,
        CTSIE = 10,
        CTSE = 9,
        RTSE = 8,
        DMAT = 7,
        DMAR = 6,
        SCEN = 5,
        NACK = 4,
        HDSEL = 3,
        IRLP = 2,
        IREN = 1,
        EIE = 0,

        // USART_GTPR
        GT  = 8,
        PSC = 0,

        // USART_RTOR
        BLEN = 24,
        RTO = 0,

        // USART_RQR
        TXFRQ = 4,
        RXFRQ = 3,
        MMRQ = 2,
        SBKRQ = 1,
        ABRRQ = 0,

        // USART_ISR
        TXFT = 27,
        RXFT = 26,
        TCBGT = 25,
        RXFF = 24,
        TXFE = 23,
        REACK = 22,
        TEACK = 21,
        WUF = 20,
        RWU = 19,
        SBKF = 18,
        CMF = 17,
        BUSY = 16,
        ABRF = 15,
        ABRE = 14,
        UDR = 13,
        EOBF = 12,
        RTOF = 11,
        CTS = 10,
        CTSIF = 9,
        LBDF = 8,
        TXFNF = 7,
        TC = 6,
        RXFNE = 5,
        IDLE = 4,
        ORE = 3,
        NE = 2,
        FE = 1,
        PE = 0,

        // USART_ICR
        WUCF = 20,
        CMCF = 17,
        UDRCF = 13,
        EOBCF = 12,
        RTOCF = 11,
        CTSCF = 9,
        LBDCF = 8,
        TCBGTCF = 7,
        TCCF = 6,
        TXFECF = 5,
        IDLECF = 4,
        ORECF = 3,
        NECF = 2,
        FECF = 1,
        PECF = 0,
    };

    enum StopBits {
        SB_1   = 0,            // 1
        SB_0_5 = 1,            // 0.5 (smartcard mode only)
        SB_2   = 2,            // 2
        SB_1_5 = 3             // 1.5
    };

    enum FifoThreshold {
        FT_1_8    = 0,          // 1/8 of its depth available (TX) or in use (RX)
        FT_1_4    = 1,          // 1/4
        FT_1_2    = 2,          // 1/2
        FT_3_4    = 3,          // 3/4
        FT_7_8    = 4,          // 7/8
        FT_EMPTY  = 5           // 8/8 empty (TX) or full (RX)
    };


private:
    typedef Ring<SEND_BUF_SIZE> SendQ;
    typedef Ring<RECV_BUF_SIZE> RecvQ;

    const uintptr_t _base;
    SendQ           _sendq;
    RecvQ           _recvq;
    mutable Mutex   _w_mutex;
    uint8_t         _read_wait; // Readers waiting
    bool            _ienable;   // Enable interrupts

    // For DMA
    Stm32Dma*       _dma;       // TX DMA is enabled if this != NULL
    uint16_t        _tx_size;   // Current DMA TX length

public:
    Stm32Usart(const uintptr_t base,
               Stm32Dma::Target txtarg,
               Stm32Dma::Target rxtarg,
               uint32_t streams)
        : Peripheral(base + (uint32_t)Register::USART_TDR,
                     base + (uint32_t)Register::USART_RDR,
                     txtarg, rxtarg, streams),
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
    void InitAsync(uint32_t baudrate, StopBits stopbit, uint32_t timer_freq, bool rtscts) {
        volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
        volatile uint32_t& cr2 = reg<volatile uint32_t>(Register::USART_CR2);
        volatile uint32_t& cr3 = reg<volatile uint32_t>(Register::USART_CR3);
        volatile uint32_t& brr = reg<volatile uint32_t>(Register::USART_BRR);
        volatile uint32_t& presc = reg<volatile uint32_t>(Register::USART_PRESC);


        uint32_t brr_val = timer_freq/(16*baudrate);
        assert(brr_val <= 0xffff);


        Thread::IPL G(IPL_UART);

        // Basically, no parity, 8 data bits, little endian, 1 stop bit
        // FIFO enable
        // TX threshold 3/4
        // RX threshold 3/4
        // Interrupt at FIFO threshold
        // CTS in input, RTS is output

        cr1 &= ~BIT(UE);

        cr1 = Bitfield(cr1)
            .cbit(RXFFIE).cbit(TXFEIE).cbit(M1).cbit(M0).cbit(EOBIE).cbit(RTOIE)
            .cbit(DEAT).cbit(DEDT).cbit(OVER8).cbit(CMIE).cbit(MME).cbit(WAKE).cbit(PCE)
            .cbit(PS).cbit(PEIE).cbit(TXFNFIE).cbit(TCIE).cbit(RXFNEIE).cbit(IDLEIE)
            .cbit(TE).cbit(RE)
            .bit(FIFOEN).bit(UESM);

        cr2 = Bitfield(cr2)
            .f(7, ADD, 0)
            .cbit(RTOEN).cbit(ABREN).cbit(MSBFIRST).cbit(DATAINV).cbit(TXINV).cbit(RXINV)
            .cbit(SWAP).cbit(LINEN).cbit(CLKEN).cbit(CPOL).cbit(CPHA).cbit(LBCL)
            .cbit(LBDIE).cbit(LBDL).cbit(ADDM7).cbit(DIS_NSS).cbit(SLVEN)
            .f(2, STOP, stopbit);

        cr3 = Bitfield(cr3)
            .f(3, TXFTCFG, FT_3_4)
            .bit(RXFTIE)
            .f(3, RXFTCFG, FT_3_4)
            .bit(TXFTIE)
            .bit(CTSE, rtscts)
            .bit(RTSE, rtscts);

        presc = 0;
        brr = brr_val;

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

                if (_dma && !_tx._active)
                    _dma->AssignTx(this);

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
    void EnableDmaTx(Stm32Dma& dma, uint8_t stream, Stm32Dma::Priority prio) {
        assert(stream <= 7);

        Mutex::Scoped L(_w_mutex);
        Thread::IPL G(IPL_UART);

        _dma = &dma;
        Peripheral::_ipl = IPL_UART;
        Peripheral::_prio = prio;
        Peripheral::_word_size = Stm32Dma::WordSize::BYTE;
        Peripheral::_tx._active = false;
        Peripheral::_trbuff = true;

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
        volatile uint32_t& cr3 = reg<volatile uint32_t>(Register::USART_CR3);
        if (enable) {
            cr1 = Bitfield(cr1)
                .bit(TXFEIE, _dma)
                .bit(RXFFIE);

            cr3 = Bitfield(cr3)
                .bit(TXFTIE, _dma)
                .bit(RXFTIE);
        } else {
            cr1 = Bitfield(cr1)
                .cbit(TXFEIE)
                .cbit(RXFFIE);

            cr3 = Bitfield(cr3)
                .cbit(TXFTIE)
                .cbit(RXFTIE);
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
        volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
        volatile uint32_t& cr3 = reg<volatile uint32_t>(Register::USART_CR3);
        volatile uint32_t& isr = reg<volatile uint32_t>(Register::USART_ISR);
        volatile uint32_t& tdr = reg<volatile uint32_t>(Register::USART_TDR);

        if (_dma) {
            if (!_sendq.Empty()) {
                if (!_tx._active)
                    _dma->Transmit(this, _sendq.Buffer(), (_tx_size = _sendq.Continuous()),
                                   true);
            } else {
                _sendq.Clear();     // Normalize
                _dma->ReleaseTx(this);
            }

            // Just in case we got here on a TXE interrupt, right after enabling DMA
            cr1 &= ~BIT(TXFEIE);
            cr3 &= ~BIT(TXFTIE);
            return;
        }

        // Not DMA - use interrupt based Tx

        // The TXE interrupt is a bit... unusual.  When TXE is high it interrupts continuously
        // and if we don't have anything to send the only way to make it stop is to disable
        // interrupts.  This makes for some rather messy code.
        if (isr & BIT(TXFT)) {
            if (!_sendq.Empty()) {
                while (!_sendq.Empty() && (isr & BIT(TXFNF)))
                    // While something to TX and FIFO not full: keep stuffing
                    tdr = _sendq.PopFront();
                    
                if (_ienable) {
                    cr1 |= BIT(TXFEIE);
                    cr3 |= BIT(TXFTIE);
                }
            } else {
                // Nothing to send, disable if TXFIFO is empty
                if (isr & BIT(TXFE)) {
                    cr1 &= ~BIT(TXFEIE);
                    cr3 &= ~BIT(TXFTIE);
                }
            }
        }
    }

    inline void HandleInterrupt() {
        volatile uint32_t& isr = reg<volatile uint32_t>(Register::USART_ISR);
        volatile uint32_t& rdr = reg<volatile uint32_t>(Register::USART_RDR);

        if (isr & BIT(TXFT)) {
            StartTx();
            if (_sendq.Empty())
                Thread::WakeSingle((void*)&_sendq);
        }

        if (isr & BIT(RXFNE)) {
            while (_recvq.Headroom() && (isr & BIT(RXFNE)))
                _recvq.PushBack(rdr);

            if (_read_wait)
                Thread::WakeSingle((void*)&_recvq);
        }
    }
};

#endif // __STM32_USART_H__
