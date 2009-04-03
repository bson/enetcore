#include "enetkit.h"
#include "ethernet.h"


EventObject _net_event;

Ethernet _eth0(CS8900A_BASE);


namespace BufferPool {

static Spinlock _lock;

static Vector<IOBuffer*> _pool;

void Initialize(uint num)
{
	Spinlock::Scoped L(_lock);

	_pool.Reserve(num);
	for (uint i = 0; i < num; ++i)
		_pool.PushBack(new IOBuffer(1600));
}


IOBuffer* Alloc()
{
	Spinlock::Scoped L(_lock);

	if (_pool.Empty()) return NULL;

	IOBuffer* buf = _pool.Back();
	_pool.PopBack();
	return buf;
}


void FreeBuffer(IOBuffer* buf)
{
	Spinlock::Scoped L(_lock);

	_pool.PushBack(buf);
}


} // namespace BufferPool


// Note, 0100-01FF are set aside as experimental
// LSB must be 0 (1 implies multicast/logical address)
// This number is chosen to be easy to type
// * static
uint16_t Ethernet::_macaddr[3] = { 0x0177, 0x4455, 0x3132 };

// Broadcast address
uint16_t Ethernet::_bcastaddr[3] = { 0xffff, 0xffff, 0xffff };


Ethernet::Ethernet(uint32_t base)
{
	_base = (volatile uint16_t*)base;
	_pp._base = _base;
}


void Ethernet::Initialize()
{
	Spinlock::Scoped L(_lock);

	_sendq.Reserve(16);
	_recvq.Reserve(16);

	// Wait for initialization to finish
	while (!(_pp[ETH_PP_SelfST] & 80)) continue;

	// RxCFG: RxOKiE
	_pp[ETH_PP_RxCFG] = 0x100;

	// RxCTL: RxOKA | IndividualA | BroadcastA
	_pp[ETH_PP_RxCTL] = 0x100|0x400|0x800;

	// TxCFG: TxOKiE
	_pp[ETH_PP_TxCFG] = 0x100;

	// BufCFG: Rdy4TxiE
	_pp[ETH_PP_BufCFG] = 0x100;

	_pid = _pp[ETH_PP_PID] | (_pp[ETH_PP_PID+2] << 16);

	// INTR: INTR0
	_pp[ETH_PP_INTR] = 0;

	// BusCLT: EnableRQ (master interrupt enable)
	_pp[ETH_PP_BusCTL] = 0x8000;

	// Hash filter
	_pp[ETH_PP_LAF] = 0;

	// IA
	_pp[ETH_PP_IA + 0] = _macaddr[0];
	_pp[ETH_PP_IA + 2] = _macaddr[1];
	_pp[ETH_PP_IA + 4] = _macaddr[2];

	_tx_state = TX_IDLE;
	_link_status = false;
	_10bt = false;

	// Last of all enable Tx, Rx
	// LineCTL: SerTxON, SerRxON, 10BT
	_pp[ETH_PP_LineCTL] = 0x40|0x80;
}


IOBuffer* Ethernet::Recv()
{
	Spinlock::Scoped L(_lock);
	if (_recvq.Empty()) return NULL;

	IOBuffer* buf = _recvq.Front();
	_recvq.PopFront();
	_recvq.Compact();
	return buf;
}


void Ethernet::Send(IOBuffer* buf)
{
	Spinlock::Scoped L(_lock);

	if (!memcmp(buf + 2, _macaddr, 6)) {
		// Loopback - just put add it to the receive queue
		_recvq.PushBack(buf);
		_net_event.Set();
		return;
	}

	_sendq.PushBack(buf);

	while (_tx_state == TX_IDLE && !_sendq.Empty())
		BeginTx();
}


// * static __irq NAKED
void Ethernet::Interrupt()
{
	SaveStateExc(4);

	// XXX check EINT2 channel (active HIGH for CS8900)
	_eth0.HandleInterrupt();

	_vic.ClearPending();
	LoadStateReturnExc();
}


void Ethernet::HandleInterrupt()
{
	Spinlock::Scoped L(_lock);

	// Update link status
	uint16_t linkst = _pp[ETH_PP_LineST];
	const bool newst = (linkst & 0x80) != 0;
	const bool new10bt = (linkst & 0x200) != 0;
	if (newst != _link_status || new10bt != _10bt) {
		_link_status = newst;
		_10bt = new10bt;
		_net_event.Set();
	}

	for (;;) {
		const uint16_t ev = _base[ETH_ISQ];
		if (!ev) return;

		// Switch on regnum
		switch (ev & 0x1f) {
		case ETH_R_RxEvent:
			ReceiveFrame(ev);
			break;

		case ETH_R_TxEvent:
			if (ev & (0x100|0x200|0x400|0x800)) {
				// Tx finished (or failed)
				_tx_state = TX_IDLE;
				do {
					BeginTx();
				} while (_tx_state == TX_IDLE);
			}
			break;

		case ETH_R_BufEvent:
			if (ev & 0x200) {
				// Underrun - send next frame (this one is lost already)
				_tx_state = TX_IDLE;
				do {
					BeginTx();
				} while (_tx_state == TX_IDLE);
			}
			else if (ev & 0x100) CopyTx(); // Buffer ready - copy and transmit
			break;

		case ETH_R_RxMISS:
		case ETH_R_TxCOL:
			// These are for stats tracking and indicate counters are halfway
			// to rolling over.
			// We don't keep stats.
			;
		}
	}
}


void Ethernet::ReceiveFrame(uint16_t rxev)
{
	_lock.AssertLocked();

	if (rxev & 0x100) {			// RxOK

		IOBuffer* buf;
		if (!_recvq.Headroom() || !(buf = BufferPool::Alloc())) {
			// No buffer available, or recvq full: drop packet
			// XXX maybe count this
			_pp[ETH_PP_RxCFG] = 0x40; // Skip_1
			return;
		}

		const uint16_t rxstatus = _base[ETH_XD0];
		const uint16_t len = _base[ETH_XD0];
		buf->SetHead(0);
		buf->SetTail(len);

		uint16_t* p = (uint16_t*)(buf + 2);
		for (uint i = 0; i < len/2; ++i)
			*p++ = _base[ETH_XD0];
		
		if (len & 1) buf[len-1] = (uint8_t&)_base[ETH_XD0];

		// Ignore runt frames.  This is rare enough not to be worth
		// optimizing.  We don't ever see partial frames or frames
		// with a bad CRC, so this is really quite exceptional.
		if (len >= 64) {
			_recvq.PushBack(buf);
			_net_event.Set();		// Signal EventObject
		} else {
			BufferPool::FreeBuffer(buf);
		}
	}
}


void Ethernet::BeginTx()
{
	_lock.AssertLocked();

	if (_tx_state != TX_IDLE) return;

	uint16_t len;
	do {
		if (_sendq.Empty()) return;

		IOBuffer* buf = _sendq.Front();
		buf->SetHead(0);
		len = buf->Size() - 2;

		if (len < 60-14 || len > 1514) {
			_sendq.PopFront();
			BufferPool::FreeBuffer(buf);
			len = 0;
		}
	} while (!len);

	// Start after entire frame is in buffer
	// TxCMD: TxStart = 0b11 (full frame)

	// XXX The optimial TxStart depends on: CCLK, RAM wait states, and
	// CS8900 wait states.  Should calculate the optimal Tx high water
	// mark in Init(), which should receive these params as input.
	_base[ETH_TxCMD] = 0b11000000;
	_base[ETH_TxLength] = len;

	_tx_state = TX_CMD;

	// Try copying right away
	const uint16_t busst = _pp[ETH_PP_BusST];
	if (busst & 0x80) {
		// TxBidErr - something wrong with the TxCMD/TxLength
		if (!_sendq.Empty())  {
			BufferPool::FreeBuffer(_sendq.Front());
			_sendq.PopFront();
		}
		_tx_state = TX_IDLE;
		return;
	}

	// If buffer is available, move on to copy frame
	if (busst & 0x100) CopyTx();
}


void Ethernet::CopyTx()
{
	_lock.AssertLocked();

	if (!_tx_state != TX_CMD) return;

	if (_sendq.Empty()) {
		// Err... what happened to our packet?!
		DiscardTx();
		return;
	}

	IOBuffer* buf = _sendq.Front();
	_sendq.PopFront();

	// First 2 bytes are alignment padding
	buf->SetHead(2);
	const uint16_t* p = (const uint16_t*)(buf + 0);

	for (uint i = 0; i < buf->Size() / 2; ++i)
		_base[ETH_XD0] = *p++;

	if (buf->Size() & 1)
		(uint8_t&)_base[ETH_XD0] = buf->Back();

	// Return buffer to pool.  If transmission fails after we get here
	// the packet is lost.
	BufferPool::FreeBuffer(buf);
}


void Ethernet::DiscardTx()
{
	_lock.AssertLocked();

	_base[ETH_TxCMD] = 0b111000000; // Force Tx reset
	_base[ETH_TxLength] = 0;
	const uint16_t tmp = _pp[ETH_PP_BusST];

	_tx_state = TX_IDLE;
}


void Ethernet::FillForBcast(IOBuffer* buf, uint dgramlen)
{
	const uint frame_len = dgramlen + 6 * 2 + 4;
	buf->SetHead(2);
	buf->SetTail(frame_len + 2);
	memcpy(buf + 0, GetBcastAddr(), GetAddrLen());
	memcpy(buf + GetAddrLen(), GetMacAddr(), GetAddrLen());
	*(uint16_t*)(buf + GetAddrLen() * 2) = Htons(frame_len);
}
