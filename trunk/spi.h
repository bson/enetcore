#ifndef __SPI_H__
#define __SPI_H__

#define SPI0_BASE 0xe0020000
#define SPI1_BASE 0xe0030000

enum {
	SPCR = 0,
	SPSR = 4,
	SPDR = 8,
	SPCCR = 0xc,
	SPINT = 0x1c
};

class SPI {
private:
	volatile uint8_t* _base;

	Spinlock _lock;

	Deque<uint8_t> _readbuf;
	Deque<uint8_t> _writebuf;

	uint _read_count;			   // Number of bytes to read
	uint _write_lowat;			   // Write low water mark (notification point)

public:

	SPI(uint32_t base);

	void Init();
	void SetSpeed(uint hz);

	static void Interrupt() __irq NAKED;

	void HandleInterrupt();

	void Select();
	void Deselect();

	void SetReadCount(uint hiwat);
	void SetWriteMark(uint lowat);

	bool WaitRead(Time deadline);
	bool WaitWrite(Time deadline);

	uint GetNumRecv() const { Spinlock::Scoped L(_lock); return _readbuf.Size(); }
	uint GetNumSend() const { Spinlock::Scoped L(_lock); return _writebuf.Size(); }

	void SetReadBuf(uint new_size);
	void SetWriteBuf(uint new_size);

	// For easy inspection of read buffer
	uint8_t operator[](uint pos) const { Spinlock::Scoped L(_lock); return _readbuf[pos]; }
	void Erase(uint num) { Spinlock::Scoped L(_lock); _readbuf.Erase(0, num); }
	void ReadClear() { Spinlock::Scoped L(_lock); _readbuf.Erase(0, _readbuf.Size()); }

	// Send data
	void Send(const void* buf, uint numbytes);

	// Retrieve block of received data
	void Recv(void* buf, uint numbytes);
};


extern SPI _spi0;

#endif // __SPI_H__
