#ifndef __ETHERNET_H__
#define __ETHERNET_H__

#include "mutex.h"


namespace BufferPool {
	void Initialize(uint num);
	IOBuffer* Alloc();
	void FreeBuffer(IOBuffer* buf);
}


class Ethernet {
	volatile uint32_t* _base;

	Spinlock _lock;

	Deque<IOBuffer*> _sendq;
	Deque<IOBuffer*> _recvq;

	bool _link_status:1;

public:
	Ethernet(uint32_t base);

	void Initialize();
	void Send(IOBuffer* buf);

	// Return next buffer, or NULL if nothing ready.
	IOBuffer* Recv();

	// Return amount to prealloc for header
	// The ethernet header is 14 bytes, but we skip the first two
	// bytes in the packet to bring the prealloc up to 16.  This
	// keeps any following headers 32-bit aligned.
	// Similarly on the receive side.
	uint GetPrealloc() const { return 16; }
private:
	// Fill up transmitter from _sendq
	void FillTx();

	// Interrupt handler
	static void Interrupt() __irq NAKED;
	void HandleInterrupt();
};


extern Ethernet _eth0;
extern EventObject _net_event;	// Wait object for network thread

#endif // __ETHERNET_H__
