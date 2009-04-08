#ifndef __DHCP_H__
#define __DHCP_H__

#include "mutex.h"
#include "network.h"
#include "time.h"
#include "netaddr.h"
#include "ethernet.h"


struct Dhcp {

	enum State {
		STATE_RESET,		   // Initial state
		STATE_DISCOVER,		   // Waiting for response to DHCPDISCOVER
		STATE_REQUEST, // Received DHCPOFFER, sent back DHCPREQUEST, waiting for ACK
		STATE_CONFIGURED // Got DHCPACK, now configured, on timer to renew
	};


	Mutex _lock;
	Ethernet& _netif;			// Interface to configure
	Time _start;				// Time since we started DHCP process
	Time _renew;				// If _leased: time when lease expires
	uint32_t _xid;				// XID we use
	State _state;
	bool _leased;				// Currently have an address (in _curaddr)
	in_addr_t _lease;			// Currently leased addr
	in_addr_t _gw;				// Default gateway
	in_addr_t _netmask;			// Netmask
	in_addr_t _dns;				// DNS server
	String _domain;				// DNS domain
	Time _rexmit;				// Next time we retransmit
	uint8_t _backoff;			// Rexmit timer (4, 8, 16, 64 sec backoff)
	in_addr_t _server;			// In STATE_OFFER, STATE_CONFIGURED, holds server id
	uint32_t _offer_xid;		// XID in server's offer
	Time _first_request;		// Time we sent first DHCPREQUEST - lease start time

	// Ports
	enum {
		CLIENT_PORT = 68,
		SERVER_PORT = 67
	};

	// DHCP type codes
	// Typical sequence:
	// C->bcast DHCPDISCOVER
	// S->C DHCPOFFER
	// C->bcast DHCPREQUEST (echoing server name)
	// S->C DHCPACK with config
	enum Type {
		DHCPINVALID = 0,		// Guard token
		DHCPDISCOVER = 1,
		DHCPOFFER = 2,
		DHCPREQUEST = 3,
		DHCPACK = 4,
		DHCPNAK = 5,
		DHCPDECLINE = 6,
		DHCPRELEASE = 7,
//		DHCPINFORM = 8  ??? Not listed in RFC1533
	};

	// Option tags - see RFC1533
	// This is a partial list
	enum {
		TAG_PAD = 0,			// 1 byte padding
		TAG_SUBNET = 1, 		// 4 byte subnet
		TAG_GW = 3,				// List of N gateways, 4 bytes each
		TAG_NS = 6,				// List of N name servers, 4 bytes each
		TAG_NAME = 12,			// Host name, N characters
		TAG_DOMAIN = 15,		// Domain name, N characters
		TAG_IP_TTL = 23,		// Default IP TTL
		TAG_ARP_TO = 35,		// ARP cache timeout
		TAG_NTP = 42,			// List of NTP servers
		TAG_VEXT = 43,			// Vendor extension block
		TAG_DHCP_REQ_IP = 50,	// DHCP Requested IP Address
		TAG_DHCP_LEASE = 51,	// DHCP Lease Time
		TAG_DHCP_MSGTYPE = 53,	// DHCP Message Type
		TAG_DHCP_SERVER = 54,	// DHCP Server identified (IP addr of server)
		TAG_DHCP_PARAM_REQ = 55, // DHCP Requested parameters
		TAG_END = 255,			// 1 byte end
	};

	// BOOTP ops
	enum { BOOTREQUEST = 1, BOOTREPLY = 2 }; // ops

	struct Packet {
		uint8_t op;
		uint8_t htype;				// 1 = ethernet
		uint8_t hlen;				// 6 for ethernet
		uint8_t hops;				// Hop counter (set to zero by client)

		uint32_t xid;				// Transaction id

		uint16_t secs;
		uint16_t flags;

		uint32_t ciaddr;

		// This is filled in by client, to use as the dest addr by server
		// We ALWAYS put 0.0.0.0 here.
		uint32_t yiaddr;

		uint32_t siaddr;
		uint32_t giaddr;
		uint8_t chaddr[16];
		uint8_t sname[64];
		uint8_t file[128];

		uint8_t options[312];


		Packet(Dhcp& dhcp) :
			// Default to ethernet boot request
			op(BOOTREQUEST), htype(1), hlen(6), hops(0),
			xid(0), secs((Time::Now() - dhcp._start).GetSec()),
			flags(0), yiaddr(0), giaddr(0)
		{
			ciaddr = 0;
			yiaddr = 0;

			memset(chaddr, 0, sizeof chaddr);
			memcpy(chaddr, dhcp._netif.GetMacAddr(), 6);
			memset(sname, 0, sizeof sname);
			memset(file, 0, sizeof file);

			options[0] = 99;		// DHCP magic cookie
			options[1] = 130;
			options[2] = 83;
			options[3] = 99;
			options[4] = TAG_END;
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

	// Signals link was recovered
	void LinkRecovered();

private:
	// Allocate packet buffer
	IOBuffer* AllocPacket(uint size);

	// Send discover message
	void SendDiscover();

	// Send request message
	void SendRequest();

	// Update retransmit timer
	void BackOff();

	// Extract config from DHCPACK
	void Extract(const uint8_t* options, uint len);

	// Get message type and server id from DHCP tag mess
	Type GetMsgType(IOBuffer* buf, in_addr_t& server);

	// Fill in DHCP packet from scratch - UDP, IP, MAC
	void FillHeader(IOBuffer* buf, const NetAddr& src, const NetAddr& dst);
};


extern Dhcp _dhcp0;

#endif // __DHCP_H__
