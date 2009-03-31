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

Ethernet::Ethernet(uint32_t base)
{
	_base = (volatile uint16_t*)base;
}


uint16_t Ethernet::GetPacketPage(uint addr)
{
	_base[ETH_PP] = addr;
	return _base[ETH_PPDATA0];
}


void Ethernet::SetPacketPage(uint addr, uint16_t val)
{
	_base[ETH_PP] = addr;
	_base[ETH_PPDATA0] = val;
}


void Ethernet::Initialize()
{
	Spinlock::Scoped L(_lock);

	// Wait for initialization to finish
	while (!(GetPacketPage(ETH_PP_SelfST) & 80)) continue;

	// RxCFG: RxOKiE
	SetPacketPage(ETH_PP_RxCFG, 0x100);

	// RxCTL: RxOKA | IndividualA | BroadcastA
	SetPacketPage(ETH_PP_RxCTL, 0x100|0x400|0x800);

	// TxCFG: TxOKiE
	SetPacketPage(ETH_PP_TxCFG, 0x100);

	// BufCFG: Rdy4TxiE
	SetPacketPage(ETH_PP_BufCFG, 0x100);

	_pid = GetPacketPage(ETH_PP_PID) | (GetPacketPage(ETH_PP_PID+2) << 16);

	// INTR: INTR0
	SetPacketPage(ETH_PP_INTR, 0);

	// BusCLT: EnableRQ (master interrupt enable)
	SetPacketPage(ETH_PP_BusCTL, 0x8000);

	// Hash filter
	SetPacketPage(ETH_PP_LAF, 0);

	// IA
	SetPacketPage(ETH_PP_IA + 0, _macaddr[0]);
	SetPacketPage(ETH_PP_IA + 2, _macaddr[1]);
	SetPacketPage(ETH_PP_IA + 4, _macaddr[2]);

	_tx_state = TX_IDLE;
	_link_status = false;
	_10bt = false;

	// Last of all enable Tx, Rx
	// LineCTL: SerTxON, SerRxON, 10BT
	SetPacketPage(ETH_PP_LineCTL, 0x40|0x80);
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
	uint16_t linkst = GetPacketPage(ETH_PP_LineST);
	const bool newst = (linkst & 0x80) != 0;
	const bool new10bt = (linkst & 0x200) != 0;
	if (newst != _link_status || new10bt != _10bt) {
		_link_status = newst;
		_10bt = new10bt;
		_net_event.Set();
	}

	for (;;) {
		uint16_t ev = _base[ETH_ISQ];
		if (!ev) return;

		// Switch on regnum
		switch (ev & 31) {
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
				// Underrun - send next frame (this one is gone already)
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

	if (rxev & 0x100) {
		// RxOK
		IOBuffer* buf = BufferPool::Alloc();
		if (!buf) return;		// No buffer available: drop packet

		const uint16_t rxstatus = _base[ETH_XD0];
		const uint16_t len = _base[ETH_XD0];
		buf->SetHead(0);
		buf->SetTail(len);

		uint16_t* p = (uint16_t*)(buf + 2);
		for (uint i = 0; i < len/2; ++i)
			*p++ = _base[ETH_XD0];
		
		if (len & 1) buf[len-1] = (uint8_t&)_base[ETH_XD0];

		_lock.Lock();
		_recvq.PushBack(buf);
		_lock.Unlock();

		_net_event.Set();		// Wake network thread
	}
}


void Ethernet::BeginTx()
{
	_lock.AssertLocked();

	uint16_t len;

	if (_tx_state != TX_IDLE) return;

	do {
		if (_sendq.Empty()) return;

		IOBuffer* buf = _sendq.Front();
		buf->SetHead(0);
		len = buf->Size() - 2;

		if (len < 60-14 || len > 1514) {
			_sendq.PopFront();
			len = 0;
		}
	} while (!len);

	// Start after entire frame is in buffer
	// TxCMD: TxStart = 0b11 (full frame)
	_base[ETH_TxCMD] = 0b11000000;
	_base[ETH_TxLength] = len;

	_tx_state = TX_CMD;

	// Try copying right away
	const uint16_t busst = GetPacketPage(ETH_PP_BusST);
	if (busst & 0x80) {
		// TxBidErr
		if (!_sendq.Empty())  _sendq.PopFront();
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

	// Return buffer to pool
	BufferPool::FreeBuffer(buf);
}


void Ethernet::DiscardTx()
{
	_base[ETH_TxCMD] = 0b111000000;
	_base[ETH_TxLength] = 0;
	GetPacketPage(ETH_PP_BusST);

	_tx_state = TX_IDLE;
}
