// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __IP_H__
#define __IP_H__

#include "mutex.h"


// Checksum
uint16_t ipcksum(const uint16_t* block, uint len, uint32_t sum = 0);

#include "icmp.h"


// IP protocols of interest
enum IpProto {
	IPPROTO_ICMP =1,
	IPPROTO_TCP = 6,
	IPPROTO_UDP = 17
};


// ARP header (fixed portion only)
enum ArpMac { ARPMAC_ETHER	= 1 };

enum ArpOp {
	ARPOP_REQ = 1,				// ARP request
	ARPOP_REP = 2,				// ARP reply
	RARPOP_REQ	= 3,			// RARP request
	RARPOP_REP = 4				// RARP reply
};

struct ArpFixed {
	uint16_t macfmt;			// MAC addr format, ARPMAC_xxx
	uint16_t protofmt;			// Protocol addr format, always IP
	uint8_t maclen;				// MAC addr length, 6 for ethernet
	uint8_t protolen;			// Protocol addr len, always 4
	uint16_t op;				// Operation, ARPOP_xxx
};

// Ethernet/IP ARP/RARP
struct Arph {
	ArpFixed fixed;				// Fixed portion of header
	uint8_t sea[6];				// Sender's ethernet address
	uint8_t sip[4];				// Sender's IP address
	uint8_t dea[6];				// Destination's ethernet address
	uint8_t dip[4];				// Destination's IP address
};



// IP header
struct __novtable Iph {
	enum Flags {
		IPFLAG_DF = 0x4000,		// Don't fragment - used for PMTU
		IPFLAG_MF = 0x2000		// More fragments
	};

	uint8_t hl_v;				// Header length and version
	uint8_t tos;
	uint16_t len;
	uint16_t id;
	uint16_t off;				// Offset & flags
	
	uint8_t ttl;
	uint8_t proto;
	uint16_t sum;
	in_addr_t source;
	in_addr_t dest;

	uint GetHLen() const { return (hl_v & 0xf) * 4; }
	uint GetVersion()  const { return (hl_v >> 4) & 0xf; }

	// Set header length; also sets version to 4.
	void SetHLen(uint hlen)  {
		assert_bounds(hlen <= 15*4);
		assert_bounds((hlen & 3) == 0);
		hl_v = 0x40 | ((hlen/4) & 0xf);
	}

	uint GetOffset() const { return (off/8) & 0x1fff; } // NBO
	uint GetFlags() const { return off & 7; }

	// Get transport header
	uint8_t* GetTransport() { return (uint8_t*)this + GetHLen(); }

	// Set checksum
	void SetCsum() {
		sum = 0;
		sum = Htons(~ipcksum((const uint16_t*)this, GetHLen()));
		if (!sum) 
            --sum;
	}

	// Get pseudo header checksum, for UDP/TCP.
	// Returns value in host byte order, as an unfolder 32-bit intermediate.
	uint32_t SumPH() const {
		return Ntohl(source) + Ntohl(dest) + proto + Ntohs(len);
	}

	bool ValidateCsum() {
		const int16_t tmp = exch<uint16_t>(sum, 0);
		const bool equal = tmp == Htons(~ipcksum((const uint16_t*)this, GetHLen()));
		sum = tmp;
		return equal;
	}
};


// Checksummer interface.  Calculates transport checksum.
class Checksummer {
public:
	// Called with buf head set to IP header
	virtual void Checksum(IOBuffer* buf) const = 0;
};


// A dummy checksummer that can be passed in to Ip::Send when the
// transport doesn't need the IP header to calculate the checksum.
class DummyChecksummer: public Checksummer {
public:
	void Checksum(IOBuffer*) const { }
};


// This IP implementation is pretty much a routing table manager.
//
// The values in the IP routing table are used as follows:
// Network Route:
//    dest=destination network & netmask
//    netmask=subnet mask of destination network
//    nexthop=router address
//    macaddr=MAC addr of router
//    expire=time MAC addr expires (rearp timer)
//    type=TYPE_RT
//
// Host Route (via router)
//    dest=destination host
//    netmask=255.255.255.255
//    nexthop=router address
//    macaddr=MAC addr of router
//    expire=time route (and MAC addr) expires
//    type=TYPE_HOSTRT
//
//  Host Route (local subnet)
//    dest=destination host
//    netmask=255.255.255.255
//    nexthop=destination host
//    macaddr=MAC addr of destination host
//    expire=time host route (and MAC addr) expires
//    type=TYPE_HOSTRT
//
// Interface:
//    dest=interface net (addr & netmask)
//    netmask=interface subnet
//    nexthop=interface addr
//    macaddr=MAC addr of interface
//    expire=<unused>
//    macvalid=true
//    type=TYPE_IF
//
// In the future we may wish to set expire to the DHCP timer for an interface.
// If an interface has multiple addresses then it has multiple routing entries.
// All interfaces are at the start of the table.
// All network routes follow.
// All route hosts then follow last.
// Lookups are *last to first*.  First match is used.

// send_ip(socket, packet):
//   DEST=packet.dest
//   for each ROUTE in table.reverse():
//      if ROUTE.dest = DEST & ROUTE.mask :
//        if ROUTE.macvalid:
//          packet.mac_dest = route.macaddr
//          if ROUTE.type = TYPE_RT:
//            ROUTE=new Route(TYPE_HOSTRT, packet.dest))
//            routes.pushback(ROUTE)
//            pending_arp.pushback(packet)
//            send_arp(ROUTE.nexthop)
//          else: if ROUTE.type = TYPE_IF:
//            loopback(packet)
//          else:
//            netif.send(packet)
//        else:
//          pending_arp.pushback(packet)
//          send_arp(ROUTE.nexthop)
//   else:
//     packet.discard()
//     socket.error(NO_ROUTE_TO_HOST)
//
// receive_ip(packet):
//   DEST=packet.dest
//   SOURCE=packet.source
//   HOSTRT=
//   NETIF=
//   for each ROUTE(TYPE_IF) in table.reverse():
//     if ROUTE.dest = DEST & ROUTE.mask:
//       if ROUTE.type = TYPE_HOSTRT:
//         ROUTE.expire.reset()
//         ROUTE.macaddr = packet.mac_source
//         HOSTRT=ROUTE
//         release_pending()
//       else: if ROUTE.type = TYPE_IF:
//         NETIF=ROUTE
//         break
//
//  if NETIF=null :
//     icmp_error(packet, NO_ROUTE_TO_HOST)
//     packet.discard()
//     return
//
//  if HOSTRT=null :
//    HOSTRT=new Route(TYPE_HOSTRT, packet.dest)
//    HOSTRT.macaddr = packet.mac_source
//    HOSTRT.expire.reset()
//    routes.pushback(HOSTRT)
//    release_pending()
//
// IPPROTO=packet.proto
// IPPROTO.receive(packet)
//
// receive_arp_reply(packet):
// ARP_HOST=packet.host_arp_is_for
// ARP_MACADDR=packet.macaddr_for_host
// for ROUTE in table.reverse():
//   if ROUTE.dest = ARP_HOST & ROUTE.mask:
//     ROUTE.macaddr = ARP_MACADDR
//     ROUTE.expire.reset()
// release_pending()
//
// release_pending:
//   for PACKET in pending_arp:
//     DEST=PACKET.dest
//     FOUND=false
//     for ROUTE in table.reverse():
//       if ROUTE.dest & ROUTE.mask = DEST & ROUTE.mask:
//         FOUND=true
//         if ROUTE.macvalid:
//             PACKET.mac_dest = ROUTE.macaddr
//             netif.send(PACKET)
//             break ROUTE
//     if FOUND:
//       pending_arp.remove(PACKET)

enum { HOSTRT_EXPIRE = 120 };
enum { ARP_LIMIT = 5 };			// Number of times we try to ARP

class Ip {

public:
	// Interface/Route/ARP table entry.  
	// Currently Ethernet centric
	struct Route {
		enum Type {
			TYPE_IF,			// Interface
			TYPE_RT,			// Route
			TYPE_HOSTRT,		// Host route/ARP entry
		};

		Ethernet& netif;		// Driver
		in_addr_t dest;			// Route/ARP dest, or IFaddr
		in_addr_t netmask;
		in_addr_t nexthop;		// Next hop
		Time expire;			// For TYPE_ARP, entry expiration or rexmit if !macvalid
		uint8_t type;			// Type of entry
		uint8_t macaddr[6];
		uint8_t arpcount;		// Number of times we've ARPed
		uint8_t refcount;

		Route* ifroute;			// Points back to TYPE_IF Route for netif

		Time lasticmp;			// Last time we sent an ICMP message
		uint16_t pmtu;

		bool macvalid:1;		// Mac addr is valid
		bool invalid:1;			// Route has been removed from table


		Route(Ethernet& n, Type t, in_addr_t addr, in_addr_t mask = 0xffffffff) :
			netif(n)
		{
			refcount = 1;
			dest = addr & mask;
			netmask = mask;
			nexthop = addr;
			type = t;
			memset(macaddr, 0, sizeof macaddr);
			macvalid = false;
			invalid = false;
			ifroute = NULL;
			pmtu = 1520;
		}

		void Retain() {
            ScopedNoInt G;

            assert(refcount);
            ++refcount;
        }
		void Release() {
            ScopedNoInt G;

			assert(refcount);

			if (!--refcount) {
                G.Unlock();
                delete this;
            }
		}

		// Ordering for rexmit list.  True if *ra > *rb
		static bool Order(const void* ra, const void* rb);
	};
private:

	mutable Mutex _lock;

	class Udp& _udp;					// UDP

	Vector<Route*> _routes;
	Vector<IOBuffer*> _pending_arp;
	Set<Route*, Route::Order> _timer; // Route timer list

	uint16_t _id;				// ID counter

public:
	Ip(Udp& udp) : _udp(udp) { }

	void Initialize();
	
	// Remove interface and all entries that refer to it
	void RemoveInterface(Ethernet& nic);

	void AddInterface(Ethernet& nic, in_addr_t addr, in_addr_t mask);
	void AddDefaultRoute(Ethernet& nic, in_addr_t router);
	void AddNetRoute(Ethernet& nic, in_addr_t network, in_addr_t netmask, in_addr_t router);

	// Go through pending ARP queue
	void ReleasePendingARP();

	// Get IP header from IOBuffer
	static Iph& GetIph(IOBuffer* buf) {
		buf->SetHead(2);        // XXX
		assert_bounds(buf->Size() >= 16 + sizeof (Iph));
		return *(Iph*)(*buf + 16);
	}

	// Set destination Mac address
	static void SetMacDest(IOBuffer* buf, uint8_t macaddr[6]);

	// Send packet.  If the Route is known, pass it in.  Returns the
	// Route used.  If caller wants to keep the Route, then the
	// refcount should be incremented.  If Route isn't known, pass in
	// NULL.  May return NULL if packet wasn't sendable (no route), in
	// which case the packet is also returned to pool.  May return a
	// route different from the one passed in, if so the route's
	// refcount has been decremented.
	//
	// The purpose of this scheme is to keep TCP and connected UDP
	// sockets from having to do a route lookup for every packet sent.
	// A transport that doesn't wish to cache Route lookups can pass in
	// NULL and ignore the result, or treat it as a boolean to indicate
	// whether the packet was routable or not.
	//
	// The canonical pattern for route caching is:
	//    Route* _cached_route;
	//
	//    // ... initialize ...
	//    _cached_route = NULL;
	//
	//    // ... send packet ...
	//    Route* r = _ip.Send(packet, _cached_route);
	//    if (r != _cached_route) {
	//      if (r)  r->Retain();
	//      _cached_route = r;
	//    }
	//    if (!r)  { ...EHOSTUNREACH... }
	//
	//   // ... teardown ..
	//   if (_cached_route)  _cached_route->Release();
	//
	// The route returned should only differ from the previous route
	// in exceptional cases (e.g. DHCP config param values changed,
	// because host was physically reconnected to a different network,
	// or a route/interface was manually reconfigured).
	//
	// tcsum is used to fill in the checksum and is passed buf (or a copy
	// of buf) with head set to point to the IP header.
	//
	// XXX Doesn't currently take into account the source address when
	// selecting an interface.  It's always treated as if it were
	// INADDR_ANY.
	//
	// df indicates whether the don't-fragment flag should be set (for PMTU)
	Route* Send(IOBuffer* buf, in_addr_t dest, const Checksummer& tcsum,
				Route* rt = NULL, bool df = false);

	// Receive for ETHERTYPE_IP
	void Receive(IOBuffer* packet);

	// Receive for ETHERTYPE_ARP
	void ArpReceive(IOBuffer* packet);
	
	// Return header ID
	uint16_t GetId() { return ++_id; }

	// Send ICMP message
	void IcmpSend(in_addr_t dest, Icmph::Type type, uint code, IOBuffer* packet);

	// Get next service time
	Time GetServiceTime() const;

	// Service entry - returns next service time
	Time Service();

private:
	// Find the Route entry for a network interface
	Route* FindIfRoute(Ethernet& netif);

	// Derive a host route derived from another entry (TYPE_IF)
	Route* AddHostRoute(in_addr_t host, Route* base);

	// Get ARP header from IOBuffer
	Arph& GetArph(IOBuffer* buf) {
		buf->SetHead(2);  // XXX
		assert_bounds(buf->Size() >= 16 + sizeof (Arph));
		return *(Arph*)(*buf + 16);
	}

	// Resolve MAC addr for Route entry
	void RequestARP(Route* rt);

	// Handle incoming ARP request
	void HandleARP(IOBuffer* buf);

	// Get source/dest mac address from frame
	uint8_t* GetMacSource(IOBuffer* buf) { return *buf + 2 + 6; }
	uint8_t* GetMacDest(IOBuffer* buf) { return *buf + 2; }
	
	// Fill in datagram with MAC header
	void FillMacHeader(IOBuffer* packet, Route* rt);

	// Fill in datagram with IP header
	void FillHeader(IOBuffer* packet, Route* rt, bool df);

	// ICMP receiver
	void IcmpReceive(IOBuffer* packet);

	// Reset Route timer
	void SetRouteTimer(Route* rt, uint secs);

	// Fill in ARP info
	void SatisfiedARP(Route* rt, const uint8_t* macaddr);
};

extern Ip _ip0;

#endif // __IP_H__
