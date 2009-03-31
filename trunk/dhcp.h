#ifndef __DHCP_H__
#define __DHCP_H__

#include "time.h"
#include "netaddr.h"


namespace Dhcp {

extern uint32_t _xid_serial;
extern Time _start;			// Time since we started DHCP process

extern State { STATE_UNBOUND, STATE_BOUND, STATE_RENEW, STATE_REBINDING };

extern State _state;
extern NetAddr _curaddr;

struct Packet {
	enum { BOOTREQUEST = 1, BOOTREPLY = 2 }; // ops

	uint8_t op;
	uint8_t htype;				// 1 = ethernet
	uint8_t hlen;				// 6 for ethernet
	uint8_t hops;				// Hop counter (set to zero by client)

	uint32_t xid;				// Transaction id

	uint16_t secs;
	uint16_t flags;

	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
	
	DhcpPacket() :
		// Default to ethernet boot request
		op(BOOTREQUEST), htype(1), hlen(6), hops(0),
		xid(++_xid_serial), secs((Time::Now() - _start).GetSecs()),
		flags(0), 
	{
	}

};


#endif // __DHCP_H__
