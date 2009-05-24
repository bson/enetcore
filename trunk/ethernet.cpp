#include "enetkit.h"
#include "ethernet.h"
#include "util.h"


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

#ifdef DEBUG
	// Read IOBASE - should be 0x300
	const uint iobase = _pp[ETH_PP_IOBASE];
	assert(iobase == 0x300);
#endif

	// Wait for initialization to finish
	while (!(_pp[ETH_PP_SelfST] & 80)) continue;

	// RxCFG: RxOKiE
	_pp[ETH_PP_RxCFG] = 0x100;

	// RxCTL: RxOKA | IndividualA | BroadcastA
	_pp[ETH_PP_RxCTL] = 0x100|0x400|0x800;

	// TxCFG: TxOKiE and all failure conditions
	_pp[ETH_PP_TxCFG] = 0x100 | 0x40 | 0x80 | 0x200 | 0x400 | 0x800 | 0x8000;

	// BufCFG: Rdy4TxiE, Tx underrun
	_pp[ETH_PP_BufCFG] = 0x100 | 0x200;

	_pid = _pp[ETH_PP_PID] | (_pp[ETH_PP_PID+2] << 16);

	// INTR: INTR0
	_pp[ETH_PP_INTR] = 0;

	// Hash filter
	_pp[ETH_PP_LAF] = 0;

	// Install Mac address
	// Use 0-0-12 for testing purposes... it's currently unassigned.
	// But check if device already appears to have a valid address; if so
	// use it.  These keeps us from generating a new mac address each time
	// we reset - and chew through the available DHCP range in no time flat.
	uint8_t buf[6];
	uint16_t* mm = (uint16_t*)buf;
	*mm++ = _pp[ETH_PP_IA + 0];
	*mm++ = _pp[ETH_PP_IA + 2];
	*mm++ = _pp[ETH_PP_IA + 4];

	if (buf[0] || buf[1] || buf[2] != 0x12) {
		_macaddr[0] = 0;
		_macaddr[1] = 0;
		_macaddr[2] = 0x12;
		_macaddr[3] = Util::Random<uint8_t>();
		_macaddr[4] = Util::Random<uint8_t>();
		_macaddr[5] = Util::Random<uint8_t>();

		const uint16_t* m = (const uint16_t*)_macaddr;

		_pp[ETH_PP_IA + 0] = *m++;
		_pp[ETH_PP_IA + 2] = *m++;
		_pp[ETH_PP_IA + 4] = *m++;
	} else {
		memcpy(_macaddr, buf, 6);
	}

	console("Ethernet: MAC address %:06h", (const uint8_t*)_macaddr);

	_tx_state = TX_IDLE;
	_link_status = false;
	_10bt = false;

	// Last of all enable Tx, Rx
	// LineCTL: SerTxON, SerRxON, 10BT
	_pp[ETH_PP_LineCTL] |= 0x40|0x80;

	const uint16_t linkst = _pp[ETH_PP_LineST];
	_link_status = (linkst & 0x80) != 0;
	_10bt = (linkst & 0x200) != 0;

	// Flush ISQ
	while (_base[ETH_ISQ] & 0x1f) continue;

	// BusCLT: EnableRQ (master interrupt enable)
	// Don't use IOCHRDY pin
	_pp[ETH_PP_BusCTL] = 0x8000 | 0x1000;
}


IOBuffer* Ethernet::Receive(uint16_t& et)
{
	Spinlock::Scoped L(_lock);
	if (_recvq.Empty()) return NULL;

	IOBuffer* buf = _recvq.Front();
	_recvq.PopFront();
	_recvq.Compact();

	buf->SetHead(0);
	uint16_t tmp;
	memcpy(&tmp, *buf + 2 + 6 + 6, 2);
	et = Ntohs(tmp);

	return buf;
}


void Ethernet::DiscardSendQ()
{
	_lock.AssertLocked();
	while (!_sendq.Empty()) {
		BufferPool::FreeBuffer(_sendq.Back());
		_sendq.PopBack();
	}
}


void Ethernet::Send(IOBuffer* buf)
{
	Spinlock::Scoped L(_lock);

	// Throw away frame if we don't have a link
	if (!_link_status) {
		DMSG("Ethernet: xmit: no link - dropping frame");
		BufferPool::FreeBuffer(buf);
		DiscardSendQ();
		return;
	}

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

	if (_vic.ChannelPending(INTCH_EINT2)) {
		_eth0.HandleInterrupt();
		_vic.DisableChannel(INTCH_EINT2);
		EXTINT = 4;					// Clear EINT2 flag
		_vic.EnableChannel(INTCH_EINT2);
	}

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

	if (!_link_status)
		DiscardSendQ();

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
				} while (!_sendq.Empty() && _tx_state == TX_IDLE);
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
		if (!_recvq.Headroom() || !(buf = BufferPool::AllocRx())) {
			// No buffer available, or recvq full: drop packet
			// XXX maybe count this
			_pp[ETH_PP_RxCFG] = 0x40; // Skip_1
			return;
		}

		const uint16_t rxstatus = _base[ETH_XD0]; // Remove Rx status frame
		const uint16_t len = _base[ETH_XD0];	  // Frame length in bytes
		buf->SetHead(2);
		buf->SetSize(len);

		// XXX inline asm frame read
		// uint16_t* readfifo_16(uint16_t* dest, volatile uint16_t* fifo, uint count);
		// uint16_t* p = readfifo_16((uint16_t*)(*buf + 0), _base + ETH_XD0, len/2);

		uint16_t* p = (uint16_t*)(*buf+0);
		for (uint i = 0; i < len/2; ++i)
			*p++ = _base[ETH_XD0];

		if (len & 1) (*buf)[len-1] = (uint8_t&)_base[ETH_XD0];

		// Ignore runt frames.  This is rare enough not to be worth
		// optimizing.  We don't ever see partial frames or frames
		// with a bad CRC, so this is really quite exceptional.
		if (len >= 64) {
			buf->SetHead(0);
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

	_base[ETH_TxCMD] = TX_START;
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

	if (_tx_state != TX_CMD) return;

	if (_sendq.Empty()) {
		// Err... what happened to our packet?!
		DiscardTx();
		return;
	}

	IOBuffer* buf = _sendq.Front();
	_sendq.PopFront();

	// First 2 bytes are alignment padding
	buf->SetHead(2);
	const uint16_t* p = (const uint16_t*)(*buf + 0);

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


void Ethernet::FillForBcast(IOBuffer* buf, uint16_t et)
{
	const uint hlen = GetAddrLen();
	memcpy(*buf + 2, GetBcastAddr(), hlen);
	memcpy(*buf + 2 + hlen, GetMacAddr(), hlen);
	et = Htons(et);
	memcpy(*buf + 2 + hlen + hlen, &et, 2);
}
