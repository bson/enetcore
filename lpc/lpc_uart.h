// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __LPC_UART_H__
#define __LPC_UART_H__


class LpcUart {
	volatile uint32_t* _base;
	Deque<uchar> _sendq;		// Tx buffer
	Deque<uchar> _recvq;		// Rx buffer
    uint8_t _irq;

    enum {
        REG_RBR = 0,
        REG_THR = 0,
        REG_DLL = 0,
        REG_DLM = 4/4,
        REG_IER = 4/4,
        REG_IIR = 8/4,
        REG_FCR = 8/4,
        REG_LCR = 0xc/4,
        REG_MCR = 0x10/4,       // Modem control, some UARTs only
        REG_LSR = 0x14/4,
        REG_SCR = 0x1c/4,
        REG_ACR = 0x20/4,
        REG_FDR = 0x28/4,
        REG_TER = 0x30/4, 
        REG_RS485CTRL = 0x4c/4,
        REG_RS485ADRMATCH = 0x50/4,
        REG_RS485DLY = 0x54/4
    };

    // LSR bits
    enum {
        LSR_RDR  = BIT0,
        LSR_OE   = BIT1,
        LSR_PE   = BIT2,
        LSR_FE   = BIT3,
        LSR_BI   = BIT4,
        LSR_THRE = BIT5,
        LSR_TEMT = BIT6,
    };

    // FCR bits
    enum {
        FCR_FIFOEN    = BIT0,
        FCR_RXFIFORES = BIT1,
        FCR_TXFIFORES = BIT2,
        FCR_DMAMODE   = BIT3,
        FCR_RXTRIGLVL_1  = BIT6 * 0,
        FCR_RXTRIGLVL_4  = BIT6 * 1,
        FCR_RXTRIGLVL_8  = BIT6 * 2, 
        FCR_RXTRIGLVL_14 = BIT6 * 3,
   };

public:
	LpcUart(uintptr_t base, uint8_t irq) :
		_base((volatile uint32_t*)base),
        _irq(irq)
	{
	}

	~LpcUart() { }

	enum {
		FRAMING_5_BITS = 0,
		FRAMING_6_BITS = 1,
		FRAMING_7_BITS = 2,
		FRAMING_8_BITS = 3,
		FRAMING_1_STOP_BIT = 0,
		FRAMING_2_STOP_BITS = 0b100,
		FRAMING_PARITY = BIT3,
		FRAMING_PARITY_NONE = 0,
		FRAMING_PARITY_ODD = FRAMING_PARITY | 0,
		FRAMING_PARITY_EVEN = FRAMING_PARITY | BIT4,
		FRAMING_PARITY_1 = FRAMING_PARITY | BIT5, // Parity, always 1
		FRAMING_PARITY_0 = FRAMING_PARITY | BIT4 | BIT5, // Parity, always 0

		FRAMING_8N1 = FRAMING_8_BITS | FRAMING_PARITY_NONE | FRAMING_1_STOP_BIT, // 8N1
	};

	void Init(uint speed, uint framing = FRAMING_8N1);

	// Send string
	void Write(const String& s);

    // Send C string
    void WriteCStr(const char* s);

	// Drain write buffer synchronously (= polled)
	void SyncDrain();

	// Interrupt handler
	static void Interrupt(void* token);

	// Enable interrupts
	void SetInterrupts(bool enable);

private:
	void FillTxFifo();
	inline void HandleInterrupt();
};


#endif // __LPC_UART_H__
