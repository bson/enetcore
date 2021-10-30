// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __NETWORK_H__
#define __NETWORK_H__

#ifdef ENABLE_IP

#include <stdint.h>
#include "core/pstring.h"
#include "core/compiler.h"
#include "core/deque.h"
#include "core/netaddr.h"
#include "core/mutex.h"

typedef Deque<uint8_t> IOBuffer;

// These are implemented elsewhere
IOBuffer* AllocNetworkBuffer();
uint8_t* AllocNetworkData(uint size, uint alignment);
uint NumNetworkBuffers(uint size);

extern EventObject _net_event; // Event object for network thread
extern Thread* _net_thread;

void* NetThread(void*);


namespace BufferPool {
	// Indicates how many buffers are always reserved for AllocRx()
	enum { TX_MIN_POOL = 4 };

	// Initialize pool with num buffer each of a specific size.
	void Initialize(uint num, uint size);

	// Allocate tx/rx buffer.  These return buffers from the same
	// pool, except the Tx variant always leaves TX_MIN_POOL behind.
	// This is to avoid using up the entire pool for transmits pending
	// receive (e.g. IP pending an ARP reply) and then have no
	// buffers left for the receive.  This is an issue when the
	// buffer pool is small.
	IOBuffer* AllocTx();
	IOBuffer* AllocRx();

	// Return buffer to pool
	void FreeBuffer(IOBuffer* buf);
}


enum EtherType {
	ETHERTYPE_IP = 0x800,
	ETHERTYPE_ARP = 0x806
};


struct InterfaceInfo {
	String _name;			// e.g. "en0"
	NetAddr _addr;			// Configured address (port == 0)
	NetAddr _mask;			// Netmask
	uint16_t _mtu;			// MTU
	bool _up:1;				// Interface is enabled
	bool _loopback:1;		// Interface is loopback
};

const Vector<InterfaceInfo*>& GetNetworkInterfaces();

// Utility function to get MAC address of first interface
bool GetIfMacAddr(const String& interface, uint8_t macaddr[6]);
bool GetMacAddr(uint8_t macaddr[6]);

#endif // ENABLE_IP

#endif // __NETWORK_H__
