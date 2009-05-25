#ifndef __MACCS8900A_H__
#define __MACCS8900A_H__

#include "mutex.h"

// CS8900A registers
enum { ETH_XD0 = 0,
	   ETH_XD1 = 1,
	   ETH_TxCMD = 2,
	   ETH_TxLength = 3,
	   ETH_ISQ = 4,
	   ETH_PP = 5,
	   ETH_PPDATA0 = 6,
	   ETH_PPDATA1 = 7,

	   // PacketPage offsets
	   ETH_PP_PID = 0,
	   ETH_PP_IOBASE = 0x20,
	   ETH_PP_INTR = 0x22,
	   ETH_PP_DMA_CH = 0x24,
	   ETH_PP_DMA_SOF = 0x26,
	   ETH_PP_DMA_FC = 0x28,
	   ETH_PP_RxDMA_BC = 0x2a,
	   ETH_PP_MemBase = 0x2c,
	   ETH_PP_PROMBase = 0x30,
	   ETH_PP_PROMMask = 0x34,
	   ETH_PP_ISQ = 0x120,
	   ETH_PP_RxCFG = 0x102,
	   ETH_PP_RxEvent = 0x124,
	   ETH_PP_RxCTL = 0x104,
	   ETH_PP_TxEvent = 0x128,
	   ETH_PP_TxCFG = 0x106,
	   ETH_PP_BufCFG = 0x10a,
	   ETH_PP_BusCTL = 0x116,
	   ETH_PP_BufEvent = 0x12c,
	   ETH_PP_RxMISS = 0x130,
	   ETH_PP_TxCOL = 0x132,
	   ETH_PP_LineCTL = 0x112,
	   ETH_PP_LineST = 0x134,
	   ETH_PP_SelfCTL = 0x114,
	   ETH_PP_SelfST = 0x136,
	   ETH_PP_BusST = 0x138,
	   ETH_PP_TestCTL = 0x118,
	   ETH_PP_TDR = 0x13c,
	   ETH_PP_TxCMD = 0x144,
	   ETH_PP_TxLength = 0x146,
	   ETH_PP_LAF = 0x150,
	   ETH_PP_IA = 0x158,

	   // Register numbers
	   ETH_R_ISQ = 0,
	   ETH_R_RxCFG = 3,
	   ETH_R_RxEvent = 4,
	   ETH_R_RxCTL = 5,
	   ETH_R_TxEvent = 8,
	   ETH_R_TxCFG = 7,
	   ETH_R_BufCFG = 0xb,
	   ETH_R_BufEvent = 0xc,
	   ETH_R_RxMISS = 0x10,
	   ETH_R_TxCOL = 0x12,
	   ETH_R_LineCTL = 0x13,
	   ETH_R_LineST = 0x14,
	   ETH_R_SelfCTL = 0x15,
	   ETH_R_SelfST = 0x16,
	   ETH_R_BusST = 0x18,
	   ETH_R_TestCTL = 0x19,
	   ETH_R_TDR = 0x1c
};


// Ethernet frame
struct EthFrame {
	uint8_t pad[2];				// Used ot align IP header to 32 bytes
	uint8_t dest[6];
	uint8_t source[6];
	uint16_t et;				// Ethertype
};

class MacCS8900a {
	volatile uint16_t* _base;

	Spinlock _lock;

	Deque<IOBuffer*> _sendq;
	Deque<IOBuffer*> _recvq;

	Eintr& _intr;				// External interrupt source

	// Transmitter state
	enum TxState {
		TX_IDLE = 0, 			// Not transmitting
		TX_CMD,					// Have issued TxCMD, TxLEN
		TX_XMIT					// Frame has been copied to chip
	};

	TxState _tx_state;

	uint32_t _pid;				// Product ID
	uint8_t _macaddr[6];		// Our Mac address
	static const uint16_t _bcastaddr[3]; // Broadcast address

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
	MacCS8900a(uint32_t base, Eintr& intr);

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

	// MTU, buffer size
	uint GetMTU() const { return 1500; }
	uint GetBufSize() const { return 1502; }

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

	// BOOTP htype
	static const uint8_t GetBootpType() { return 1; }

	// Fill in a buffer for broadcast
	void FillForBcast(IOBuffer* buf, uint16_t et);

	// Interrupt handler
	static void Interrupt() __irq __naked;
private:
	// CS8900A TxCMD codes
	// XXX The optimial TxStart depends on: CCLK, RAM wait states, and
	// CS8900 wait states.  We should calculate the optimal Tx high water
	// mark in Init(), which should pick a high-water mark.
	// When debugging, TX_STARTALL is a good idea.
	enum {
		TX_START4 = 0,			// Start after 4 bytes
		TX_START64 = 0x40,		// 64
		TX_START128 = 0x80,		// 128
		TX_STARTALL = 0xc0,		// After entire frame
		TX_START = TX_STARTALL
	};

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

	// Throw away send queue
	void DiscardSendQ();

protected:
	MacCS8900a(const MacCS8900a&);
};

#endif // __MACCS8900A_H__
