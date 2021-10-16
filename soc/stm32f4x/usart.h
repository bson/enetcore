// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __STM32_USART_H__
#define __STM32_USART_H__

#include "ring.h"
#include "mutex.h"

class Stm32Usart {
public:
    enum { 
        SEND_BUF_SIZE = 128,
        RECV_BUF_SIZE = 16
    };

private:
    const uintptr_t _base;
    Ring<SEND_BUF_SIZE> _sendq; // TX send q
    Ring<RECV_BUF_SIZE> _recvq; // RX send q
    mutable Mutex _w_mutex;
    uint8_t _read_wait;         // Readers waiting
    bool _ienable;              // Enable interrupts

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


    Stm32Usart(const uintptr_t base)
        : _base(base),
          _ienable(false),
          _read_wait(0) {
    }

    template <typename T>
    T& reg(const Register r) {
        return *((T*)(_base + (uint32_t)r)); 
    }

    // Set up for async use.  8 bits no parity.  1 or 2 stop bits.
    void InitAsync(uint32_t baudrate, StopBits stopbit, uint32_t timerclk);
    
    // Write buffer
    void Write(const uint8_t* data, uint len);

	// Write string
	void Write(const String& s) { Write(s.CStr(), s.Size()); }

    // Write C string
    void WriteCStr(const char* s) { Write((const uint8_t*)s, strlen(s)); }

    // getc
    int getc();

	// Drain write buffer synchronously (= polled)
	void SyncDrain();

	// Interrupt handler
	static void Interrupt(void* token);

	// Enable interrupts
	void SetInterrupts(bool enable);

private:
    inline void WriteByte();
	inline void HandleInterrupt();

    Stm32Usart(const Stm32Usart&);
    Stm32Usart& operator=(const Stm32Usart&);
};

#endif // __STM32_USART_H__
