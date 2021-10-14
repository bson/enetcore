// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "thread.h"
#include "ring.h"
#include "stm32f4x/usart.h"

// Set up for async use.  8 bits no parity.  1 or 2 stop bits.
void Stm32Usart::InitAsync(uint32_t baudrate, StopBits stopbit, uint32_t timerclk) {
    // BRR is Q12.4, so
    // BRR*16 = clk/(16*baud)   => BRR = clk/baud
    // XXX Simple truncatation, maybe use Q12.5 and round manually, but probably good enough (<< 1% error)
    const uint32_t div = timerclk/baudrate;
    assert(div <= 0xffff);

    Thread::IPL G(IPL_UART);

    volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
    cr1 &= BIT(PEIE) | BIT(TXEIE) | BIT(TCIE) | BIT(RXNEIE) | BIT(TE) | BIT(RE) | BIT(SBK) | BIT(RWU)
        | BIT(M) | BIT(UE);

    volatile uint32_t& cr2 = reg<volatile uint32_t>(Register::USART_CR2);
    cr2 &= ~(BIT(LINEN) | (3 << STOP) | BIT(CLKEN) | BIT(CPOL) | BIT(CPHA) | BIT(LBDIE));

    volatile uint32_t& cr3 = reg<volatile uint32_t>(Register::USART_CR3);
    cr3 = (cr3 & ~(BIT(CTSIE) | BIT(CTSE) | BIT(RTSE) | BIT(DMAT) | BIT(DMAR) | BIT(SCEN)
                   | BIT(NACK) | BIT(HDSEL) | BIT(IRLP) | BIT(IREN) | BIT(EIE)))
        | BIT(ONEBIT);
    
    reg<volatile uint32_t>(Register::USART_BRR) = div;

    cr2 |= (uint32_t)stopbit << STOP;
    
    cr1 &= BIT(OVER8);
    cr1 |= BIT(UE);
    cr1 |= BIT(TE) | BIT(RE);
}
    
// Write buffer
void Stm32Usart::Write(const uint8_t* data, uint len) {
    Mutex::Scoped L(_w_mutex);
    Thread::IPL G(IPL_UART);
    
    for (const uint8_t *p = data; p < data + len; ) {
        if (_sendq.Headroom()) {
            while (_sendq.Headroom() && (p < data + len))
                _sendq.PushBack(*p++);

            FillTd();
        }
        if (p < data + len)
            Thread::WaitFor(this);
    }
}

void Stm32Usart::FillTd() {
    volatile uint32_t& sr = reg<volatile uint32_t>(Register::USART_SR);
    if (!_sendq.Empty() && (sr & BIT(TXE)) != 0) {
        reg<volatile uint32_t>(Register::USART_DR) = _sendq.PopFront();
        if (_ienable) {
            volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
            cr1 |= BIT(TXEIE);
        }
    }
}

// Drain write buffer Stm32Usart::synchronously (= polled)
void Stm32Usart::SyncDrain() {
    ScopedNoInt G;

    while (!_sendq.Empty())
        FillTd();
}

// Enable interrupts
void Stm32Usart::SetInterrupts(bool enable) {
    _ienable = enable;

    volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
    if (!enable) {
        cr1 &= ~(BIT(TXEIE) | BIT(RXNEIE));
    } else {
        cr1 |= BIT(RXNEIE);
    }
}

inline void Stm32Usart::HandleInterrupt() {
    volatile uint32_t& sr = reg<volatile uint32_t>(Register::USART_SR);
    bool wake = false;
    
    // The TXE interrupt is a bit... unusual.  When TXE is high it interrupts continuously
    // and if we don't have anything to send the only way to make it stop is to disable
    // interrupts.  This makes for some rather messy code.  On TXE int we disable interrupts,
    // and FillTd() will when reenable it if 1) we want interrupts, and 2) we had something
    // to send.  If there is nothing to send we leave it disabled.  TXE remains set.
    if (sr & BIT(TXE)) {
        volatile uint32_t& cr1 = reg<volatile uint32_t>(Register::USART_CR1);
        cr1 &= ~BIT(TXEIE);

        FillTd();
        wake = true;
    }

    if (sr & BIT(RXNE)) {
        volatile uint32_t& dr = reg<volatile uint32_t>(Register::USART_DR);
        const uint8_t c = (uint8_t)dr;
        if (_recvq.Headroom()) {
            _recvq.PushBack(c);
            wake = true;
        }
    }
    
    if (wake)
        Thread::WakeAll(this);
}

// Interrupt handler
void Stm32Usart::Interrupt(void* token) {
    ((Stm32Usart*)token)->HandleInterrupt();
}
