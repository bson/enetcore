#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include "mutex.h"


namespace BufferPool {
	void Initialize(uint num);
	IOBuffer* Alloc();
	void FreeBuffer(IOBuffer* buf);
}


// Ethernet frame
struct EthFrame {
	uint8_t pad[2];				// Used ot align IP header to 32 bytes
	uint8_t dest[6];
	uint8_t source[6];
	uint16_t et;				// Ethertype
};

class Ethernet {
	volatile uint16_t* _base;

	Spinlock _lock;

	Deque<IOBuffer*> _sendq;
	Deque<IOBuffer*> _recvq;

	// Transmitter state
	enum TxState {
		TX_IDLE = 0, 			// Not transmitting
		TX_CMD,					// Have issued TxCMD, TxLEN
		TX_XMIT					// Frame has been copied to chip, it's sending
	};

	TxState _tx_state;

	uint16_t _pid;				// Product ID
	static uint16_t _macaddr[3]; // Our Mac address
	static uint16_t _bcastaddr[3]; // Broadcast address

	bool _link_status:1;		// Link status indicator
	bool _10bt:1;				// 10BT transceiver, otherwise AUI (yeah, right :))

	struct PacketPage {
		volatile uint16_t* _base;

		volatile uint16_t& operator[](int addr) volatile {
			_base[ETH_PP] = addr;
			return _base[ETH_PPDATA0];
		}
	};

	volatile PacketPage _pp;

public:
	Ethernet(uint32_t base);

	void Initialize();
	void Send(IOBuffer* buf);

	// Return next buffer, or NULL if nothing ready.
	// Sets et to ethertype (in host byte order)
	IOBuffer* Receive(uint16_t& et);

	// Return Link status
	bool GetLinkStatus() const { return _link_status; }

	// True of using 10BT, false is AUI (which we never use, so shouldn't happen)
	bool Get10BT() const { return _10bt; }

	// Return amount to prealloc for header
	// The ethernet header is 14 bytes, but we skip the first two
	// bytes in the packet to bring the prealloc up to 16.  This
	// keeps any following headers 32-bit aligned.
	// Similarly on the receive side.
	uint GetPrealloc() const { return 16; }

	// True if buffer dest addr is broadcast
	static bool IsBrodcast(const IOBuffer& buf) {
		// Only bother checking the first word
		return *(uint16_t*)(buf + 2) == 0xffff;
	}

	// Get our configured mac address
	const uint8_t* GetMacAddr() const { return (const uint8_t*)_macaddr; }

	// Get broadcast address
	static const uint8_t* GetBcastAddr() { return (const uint8_t*)_bcastaddr; }

	// Address length
	static const uint GetAddrLen() { return 6; }

	// Buffer pad
	static const uint GetBufPad() { return 2; }

	// Fill in a buffer for broadcast
	void FillForBcast(IOBuffer* buf, uint dgramlen);

	// Interrupt handler
	static void Interrupt() __irq NAKED;
private:
	// Begin Tx for head of sendq - issue command
	void BeginTx();

	// Buffer event received - copy frame to transmit.
	// Call when we know there is on-chip buffer space.
	void CopyTx();

	// Instance interrupt handler
	void HandleInterrupt();

	// Receive frame
	// rxev is the RxEvent contents
	void ReceiveFrame(uint16_t rxev);

	// Flush on-chip Tx buffer
	void DiscardTx();
};

extern Ethernet _eth0;

#endif // __ETHERNET_H__
