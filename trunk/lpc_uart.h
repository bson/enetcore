#ifndef __LPC_UART_H__
#define __LPC_UART_H__


class LpcUart {
	volatile uint8_t* _base;
	Deque<uchar> _sendq;		// Tx buffer
	Deque<uchar> _recvq;		// Rx buffer
	mutable Spinlock _lock;

public:
	LpcUart(uintptr_t base, uint default_speed = 9600) : 
		_base((volatile uint8_t*)base)
	{
		SetSpeed(default_speed);
	}

	~LpcUart() { }

	enum {
		FRAMING_5_BITS = 0,
		FRAMING_6_BITS = 1,
		FRAMING_7_BITS = 2,
		FRAMING_8_BITS = 3,
		FRAMING_1_STOP_BIT = 0,
		FRAMING_2_STOP_BITS = 0b100,
		FRAMING_PARITY = 0b1000,
		FRAMING_PARITY_NONE = 0,
		FRAMING_PARITY_ODD = FRAMING_PARITY | 0b000000,
		FRAMING_PARITY_EVEN = FRAMING_PARITY | 0b010000,
		FRAMING_PARITY_1 = FRAMING_PARITY | 0b100000, // Parity, always 1
		FRAMING_PARITY_0 = FRAMING_PARITY | 0b000000, // Parity, always 0

		FRAMING_8N1 = FRAMING_8_BITS | FRAMING_PARITY_NONE | FRAMING_1_STOP_BIT, // 8N1
	};

	void SetSpeed(uint speed, uint framing = FRAMING_8N1);

	// Send string
	void Write(const String& s);

	// Drain write buffer synchronously (= polled)
	void SyncDrain();

	// Interrupt handler
	static void Interrupt() __irq __naked;

	// Enable interrupts
	void SetInterrupts(bool enable);

private:
	void FillFifo();
	void HandleInterrupt();
};


#endif // __LPC_UART_H__
