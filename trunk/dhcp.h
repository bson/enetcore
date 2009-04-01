#ifndef __DHCP_H__
#define __DHCP_H__

#include "time.h"
#include "netaddr.h"
#include "ethernet.h"


struct Dhcp {

	enum State {
		STATE_RENEW,		   // Should start acquiring a lease
		STATE_DISCOVER,		   // Waiting for response to DHCPDISCOVER
		STATE_REQUEST, // Received DHCPOFFER, sent back DHCPREQUEST, waiting for ACK
		STATE_CONFIGURED // Got DHCPACK, now configured, on timer to renew
	};

#define INITIAL_XID 'enet'		// Start value for our XIDs

	Ethernet& _netif;			// Interface to configure
	uint32_t _xid_serial;
	Time _start;				// Time since we started DHCP process
	Time _renew;				// If _leased: time when lease expires
	State _state;
	bool _leased;				// Currently have an address (in _curaddr)
	NetAddr _lease;				// Currently leased addr
	NetAddr _gw;				// Default gateway
	NetAddr _netmask;			// Netmask
	NetAddr _dns;				// DNS server
	String _domain;				// DNS domain
	uint8_t _backoff;			// Rexmit timer (4, 8, 16, 64 sec backoff)

	struct Packet {
		enum { BOOTREQUEST = 1, BOOTREPLY = 2 }; // ops

		// DHCP type codes
		// Typical sequence:
		// C->bcast DHCPDISCOVER
		// S->C DHCPOFFER
		// C->bcast DHCPREQUEST (includes sname of selected sever)
		// S->C DHCPACK with config
		enum {
			DHCPDISCOVER,
			DHCPOFFER,
			DHCPREQUEST,
			DHCPACK,
			DHCPNAK,
			DHCPDECLINE,
			DHCPRELEASE,
			DHCPINFORM
		};

		uint8_t op;
		uint8_t htype;				// 1 = ethernet
		uint8_t hlen;				// 6 for ethernet
		uint8_t hops;				// Hop counter (set to zero by client)

		uint32_t xid;				// Transaction id

		uint16_t secs;
		uint16_t flags;

		uint32_t ciaddr;
		uint32_t yiaddr;			// Returned addr in DHCPOFFER
		uint32_t siaddr;
		uint32_t giaddr;
		uint8_t chaddr[16];
		uint8_t sname[64];
		uint8_t file[128];

		uint8_t options[312];


		Packet(uint type) :
			// Default to ethernet boot request
			op(BOOTREQUEST), htype(1), hlen(6), hops(0),
			xid(++_xid_serial), secs((Time::Now() - _start).GetSecs()),
			flags(0), yiaddr(0), giaddr(0)
		{
			ciaddr = _leased ? _curaddr.GetAddr4() : 0;
			yiaddr = 0;
			memcpy(chaddr, Ethernet::GetMacAddr(), 6);
			sname[0] = 0;
			file[0] = 0;
			options[0] = 99;		// DHCP magic cookie
			options[1] = 130;
			options[2] = 83;
			options[3] = 99;
			options[4] = type;
		}
	};


	Dhcp(Ethernet& netif);

	// (Re)Initialize and start obtaining config.  Called on powerup and when
	// the ethernet link is restored.
	void Reset();

	// Process incoming packet if DHCP.  DHCP packet buffers are
	// returned to the pool.  If the packet was processed this
	// function returns true, otherwise it returns false.
	bool Receive(IOBuffer* buf);

	// Get next service time
	Time GetServiceTime();

	// Service - returns next service time
	Time Service();
};


#endif // __DHCP_H__
