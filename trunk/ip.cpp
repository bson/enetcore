#include "enetkit.h"
#include "ip.h"
#include "udp.h"


Ip _ip;


uint16_t ipcksum(const uint16_t* block, uint len, uint32_t sum)
{
	const bool unaligned = len & 1;

	len /= 2;

	// XXX make this a 32-bit op on ARM
	while (len--) sum += Ntohs(*block++);

	if (unaligned)  sum += *(const uint8_t*)block << 8;

	while (sum > 0xffff)
		sum = (sum & 0xffff) + (sum >> 16);

	return sum;
}


void Ip::Initialize()
{
	_id = Util::Random<uint16_t>();
}



void Ip::AddInterface(Ethernet& nic, in_addr_t addr, in_addr_t mask) 
{
	Mutex::Scoped L(_lock);

	RemoveInterface(nic);
	Route* rt = new Route(nic, Route::TYPE_IF, addr, mask);
	rt->ifroute = rt;
	_routes.PushFront(rt);
}


void Ip::RemoveInterface(Ethernet& nic) 
{
	Mutex::Scoped L(_lock);

	for (uint i = 0; i < _routes.Size(); ) {
		Route* rt = _routes[i];
		if (&rt->netif == &nic) {
			rt->invalid = true;
			rt->Release();
			_routes.Erase(i);
		} else {
			++i;
		}
	}

	// Prune pending ARP
	ReleasePendingARP();
}


Ip::Route* Ip::FindIfRoute(Ethernet& netif)
{
	_lock.AssertLocked();

	for (uint i = 0; i < _routes.Size(); ++i) {
		Route* rt = _routes[i];
		if (rt->type != Route::TYPE_IF) break;
		if (&rt->netif == &netif)  return rt;
	}

	return NULL;
}


void Ip::AddDefaultRoute(Ethernet& nic, in_addr_t router)
{
	Mutex::Scoped L(_lock);

	Route* netif = FindIfRoute(nic);
	if (!netif) return;

	// Insert immediately following last TYPE_IF
	int pos;
	for (pos = _routes.Size() - 1; pos >= 0; --pos) {
		if (_routes[pos]->type == Route::TYPE_IF) {
			Route* rt = new Route(nic, Route::TYPE_RT, 0, 0);
			rt->nexthop = router;
			rt->ifroute = netif;
			_routes.Insert(pos + 1) = rt;
			break;
		}
	}
}


void Ip::AddNetRoute(Ethernet& nic, in_addr_t network, in_addr_t netmask, in_addr_t router)
{
	Mutex::Scoped L(_lock);

	Route* netif = FindIfRoute(nic);
	if (!netif) return;

	// Insert immediately following last TYPE_RT
	for (int pos = _routes.Size() - 1; pos >= 0; --pos) {
		if (_routes[pos]->type == Route::TYPE_RT) {
			Route* rt = new Route(nic, Route::TYPE_RT, network, netmask);
			rt->nexthop = router;
			rt->ifroute = netif;
			_routes.Insert(pos + 1) = rt;
			break;
		}
	}
}


Ip::Route* Ip::AddHostRoute(in_addr_t host, Route* rt)
{
	_lock.AssertLocked();
	assert(rt->type == Route::TYPE_IF);

	Route *hostrt = new Route(rt->netif, Route::TYPE_HOSTRT, host);
	hostrt->nexthop = host;
	hostrt->macvalid = false;
	hostrt->ifroute = rt;
	_routes.PushBack(hostrt);

	return hostrt;
}


void Ip::ReleasePendingARP()
{
	Mutex::Scoped L(_lock);

	for (uint i = 0; i < _pending_arp.Size(); ) {
		IOBuffer* packet = _pending_arp[i];
		const in_addr_t dest = GetIph(packet).dest;
		const bool is_icmp = GetIph(packet).proto == IPPROTO_ICMP;
		bool remove_packet = false;

		for (int n = _routes.Size() - 1; n >= 0; --n) {
			Route* rt = _routes[n];
			if (rt->dest == dest & rt->netmask) {
				if (rt->macvalid) {
					if (is_icmp) {
						// Throttle ICMP
						if (Time::Now() < rt->lasticmp + Time::FromSec(1)) {
							BufferPool::FreeBuffer(packet);
							remove_packet = true;
							goto done;
						}
						rt->lasticmp = Time::Now();
					}

					FillMacHeader(packet, rt);
					rt->netif.Send(packet);
					remove_packet = true;
				}
				// remove_packet = false;
				goto done;
			}
		}

		// No route found for datagram - drop it
		remove_packet = true;

	done:
		if (remove_packet) {
			if (i != _pending_arp.Size() -1)
				_pending_arp[i] = _pending_arp.Back();

			_pending_arp.PopBack();
		} else
			++i;
	}
}


void Ip::FillMacHeader(IOBuffer* buf, Route* rt)
{
	buf->SetHead(0);
	memcpy(*buf + 2, rt->macaddr, 6);
	memcpy(*buf + 2 + 6, rt->netif.GetMacAddr(), 6);
	const uint16_t et = Htons(ETHERTYPE_IP);
	memcpy(*buf + 2 + 6, &et, 2);
}


void Ip::FillHeader(IOBuffer* buf, Route* rt, bool df)
{
	FillMacHeader(buf, rt);

	// Fill in source addr
	Iph& iph = GetIph(buf);
	if (true /* iph.source == INADDR_ANY */) {
		const Route* ifroute = rt->type == Route::TYPE_IF ? rt : rt->ifroute;
		assert(ifroute);
		assert(ifroute->type == Route::TYPE_IF);
		iph.source = ifroute->nexthop;
	}

	// id = 0 means header is not yet filled in
	// Source, dest need to be filled in
	if (!iph.id)  {
		iph.id = ++_id;

		iph.SetHLen(sizeof (Iph));
		iph.tos = 0;
		iph.off = df ? Iph::IPFLAG_DF : 0;
		iph.ttl = 255;

		// Length and checksum
		iph.len = Htons(*buf + buf->Size() - (uint8_t*)&iph);
		iph.SetCsum();
	}
}


Ip::Route* Ip::Send(IOBuffer* buf, in_addr_t dest, const Checksummer& tcsum,
					Ip::Route* prevrt, bool df)
{
	Iph& iph = GetIph(buf);
	iph.dest = dest;

	Mutex::Scoped L(_lock);

	if (prevrt) {
		if (prevrt->invalid) {
			prevrt->Release();
		} else {
#if 0
			// Throttle back ICMP.  We throttle before adding to the ARP list as
			// well as when we drain it after successful ARP.
			// XXX we never get here...
			if (iph.proto == IPPROTO_ICMP) {
				if (Time::Now() < prevrt->lasticmp + Time::FromSec(1)) {
					BufferPool::FreeBuffer(buf);
					return prevrt;
				}
				prevrt->lasticmp = Time::Now();
			}
#endif
			FillHeader(buf, prevrt, df);
			buf->SetHead(prevrt->netif.GetPrealloc());
			tcsum.Checksum(buf);

			if (prevrt->macvalid) {
				prevrt->netif.Send(buf);
				return prevrt;
			} 

			// No ARP info yet.
			_pending_arp.PushBack(buf);
			return prevrt;
		}
	}

	const bool is_icmp = iph.proto == IPPROTO_ICMP;

	for (int i = _routes.Size()-1; i >= 0; --i) {
		Route* rt = _routes[i];
		if (rt->dest == dest & rt->netmask) {
			if (is_icmp) {
				// Throttle ICMP
				if (Time::Now() < rt->lasticmp + Time::FromSec(1)) {
					BufferPool::FreeBuffer(buf);
					return rt;
				}
				rt->lasticmp = Time::Now();
			}

			if (rt->macvalid) {
				FillHeader(buf, rt, df);
				buf->SetHead(rt->netif.GetPrealloc());
				tcsum.Checksum(buf);

				if (rt->type == Route::TYPE_IF) {
					if (rt->dest == dest) {
						rt->netif.Send(buf); // Loopback: ethernet driver will return it
						return rt;
					} else {
						Route* hostrt = AddHostRoute(dest, rt);
						_pending_arp.PushBack(buf);
						RequestARP(hostrt);
						return hostrt;
					}
				} else {
					rt->netif.Send(buf);
					return rt;
				}
			} else {
				_pending_arp.PushBack(buf);
				RequestARP(rt);
				return rt;
			}
		}
	}

	// No route
	BufferPool::FreeBuffer(buf);
	return NULL;
}


void Ip::Receive(IOBuffer* packet)
{
	Iph& iph = GetIph(packet);

	if (_routes.Empty() || packet->Size() < 16 + Ntohs(iph.len) || !iph.ValidateCsum()) {
		BufferPool::FreeBuffer(packet);
		return;
	}

	const in_addr_t dest = iph.dest;
	const in_addr_t source = iph.source;
	Route* hostrt = NULL;
	Route* netif = NULL;

	{
		Mutex::Scoped L(_lock);

		for (int i = _routes.Size()-1; i >= 0; --i) {
			Route* rt = _routes[i];
			if (rt->dest == dest & rt->netmask) {
				if (rt->type == Route::TYPE_HOSTRT) {
					rt->ResetExpire();
					// Glean ARP info
					memcpy(rt->macaddr, GetMacSource(packet), 6);
					rt->macvalid = true;
					hostrt = rt;
					ReleasePendingARP();
				} else if (rt->type == Route::TYPE_IF) {
					netif = rt;
					break;
				}
			}
		}

		if (!netif) {
			BufferPool::FreeBuffer(packet);
			return;
		}

		if (!hostrt) {
			hostrt = AddHostRoute(source, netif);
			memcpy(hostrt->macaddr, GetMacSource(packet), 6);
			hostrt->macvalid = true;
			ReleasePendingARP();
		}
	}

	assert(netif);
	const uint proto = GetIph(packet).proto;

	// Skip past MAC header for transport
	packet->SetHead(netif->netif.GetPrealloc());

	switch (proto) {
	case IPPROTO_ICMP:
		IcmpReceive(packet);
		break;
	case IPPROTO_UDP:
		_udp.Receive(packet);
		break;
#if 0
	case IPPROTO_TCP:
		Tcp::Receive(packet);
		break;
#endif
	default:
		IcmpSend(source, Icmph::ICMP_DEST_UNREACH, Icmph::ICMP_PROTO_UNREACH, packet);
		BufferPool::FreeBuffer(packet);
		return;
	}
}


void Ip::ArpReceive(IOBuffer* packet)
{
	Mutex::Scoped L(_lock);

	Arph& arp = GetArph(packet);
	if (Htons(arp.fixed.op) == ARPOP_REQ) {
		HandleARP(packet);
		return;
	}

	if (Htons(arp.fixed.op) != ARPOP_REP) {
		BufferPool::FreeBuffer(packet);
		return;
	}

	in_addr_t arp_addr;
	memcpy(&arp_addr, arp.sip, 4);	// Not 32 bit aligned
	const uint8_t* arp_macaddr = arp.sea;

	for (int i = _routes.Size()-1; i >= 0; --i) {
		Route* rt = _routes[i];
		if (rt->dest == arp_addr & rt->netmask) {
			memcpy(rt->macaddr, arp_macaddr, 6);
			rt->ResetExpire();
			ReleasePendingARP();
			break;
		}
	}
}


void Ip::RequestARP(Route* rt)
{
	_lock.AssertLocked();
	if (_routes.Empty()) return; // Can't ARP without a configure i/f

	int netif = -1;
	for (uint i = 0; i < _routes.Size(); ++i) {
		assert(_routes[i]->type == Route::TYPE_IF);

		if (&_routes[i]->netif == &rt->netif) {
			netif = i;
			break;
		}
	}

	assert(netif != -1);

	IOBuffer* buf = BufferPool::Alloc();
	if (!buf) return;

	buf->SetSize(16 + sizeof (Arph));
	memcpy(*buf + 2, _routes[netif]->macaddr, 6);
	memcpy(*buf + 2 + 6, rt->netif.GetBcastAddr(), 6);
	const uint16_t et = Htons(ETHERTYPE_ARP);
	memcpy(*buf + 2 + 6 + 6, &et, 2);

	Arph& arp = GetArph(buf);
	arp.fixed.macfmt = Htons(ARPMAC_ETHER);
	arp.fixed.protofmt = Htons(ETHERTYPE_IP);
	arp.fixed.maclen = sizeof arp.dea;
	arp.fixed.protolen = sizeof arp.dip;
	arp.fixed.op = Htons(ARPOP_REQ);
	memcpy(arp.sea, rt->netif.GetMacAddr(), 6);
	memcpy(arp.sip, &_routes[netif]->nexthop, 4);
	memset(arp.dea, 0, sizeof arp.dea);
	memcpy(arp.dip, &rt->nexthop, 4);

	rt->netif.Send(buf);
}


void Ip::HandleARP(IOBuffer* packet)
{
	_lock.AssertLocked();

	packet->SetHead(0);
	Arph& arp = GetArph(packet);

	if (packet->Size() < 16 + sizeof (Arph) ||
		Ntohs(arp.fixed.macfmt) != ARPMAC_ETHER ||
		Ntohs(arp.fixed.protofmt) != ETHERTYPE_IP ||
		arp.fixed.maclen != 6 || arp.fixed.protolen != 4) {

		// Nothing we can resolve
		BufferPool::FreeBuffer(packet);
		return;
	}
		
	in_addr_t arp_addr;
	memcpy(&arp_addr, arp.dip, 4);
	const uint8_t* arp_macaddr = arp.dea;

	for (uint i = 0; i < _routes.Size(); ++i) {
		Route* rt = _routes[i];
		if (rt->nexthop == arp_addr && rt->macvalid) {
			arp.fixed.op = Htons(ARPOP_REP);
			memcpy(arp.sea, rt->macaddr, 6);
			memcpy(arp.sip, &_routes[0]->nexthop, 4);
			memcpy(arp.dea, arp_macaddr, 6);
			memcpy(arp.dip, &arp_addr, 4);
			rt->netif.Send(packet);
			return;
		}
	}
}


void Ip::IcmpReceive(IOBuffer* packet)
{
	Iph& iph = GetIph(packet);
	Icmph& icmph = *(Icmph*)iph.GetTransport();

	const Icmph::Type type = (Icmph::Type)icmph.type;
	const uint code = icmph.code;

	switch (type) {
	case Icmph::ICMP_ECHO_REQ:
		IcmpSend(iph.dest, Icmph::ICMP_ECHO_REPLY, 0, packet);
		break;
	case Icmph::ICMP_DEST_UNREACH:
		Iph& iph2 = *(Iph*)icmph.GetEnclosed();

		if (icmph.GetEnclosed() + sizeof iph2 >= &packet->Back() ||
			iph2.GetHLen() < 20 ||
			icmph.GetEnclosed() + iph2.GetHLen() + sizeof (Udph) + 8 >= &packet->Back() ||
			!iph2.ValidateCsum()) {
			break;
		}

		switch (iph2.proto) {
		case IPPROTO_UDP:
			_udp.IcmpError(type, code, iph.source, iph2.dest, iph2.source,
						   *(Udph*)iph2.GetTransport());
			break;
#if 0
		case IPPROTO_TCP:
			_tcp.IcmpError(type, code, iph.source, iph2.dest, iph2.source,
						   *(Tcph*)iph2.GetTransport());
			break;
#endif
		}
	}
	
	BufferPool::FreeBuffer(packet);
}


void Ip::IcmpSend(in_addr_t dest, Icmph::Type type, uint code, IOBuffer* packet)
{
	IOBuffer* buf = BufferPool::Alloc();
	if (!buf) return;

	buf->SetHead(0);

	Iph& iph = *(Iph*)(*buf + 16);
	Icmph& icmph = *(Icmph*)iph.GetTransport();
	iph.proto = IPPROTO_ICMP;
	iph.source = INADDR_ANY;

	icmph.type = type;
	icmph.code = code;

	const uint bufspace = &buf->Back() - icmph.GetEnclosed();
	const uint toinclude = min(bufspace, packet->Size());
	assert(toinclude >= sizeof (Iph) + 16);

	memcpy(icmph.GetEnclosed(), &packet->Front(), toinclude);
	buf->SetSize((uint8_t*)icmph.GetEnclosed() - &buf->Front() + toinclude);
	icmph.SetCsum(sizeof icmph + toinclude);

	_ip.Send(buf, dest, DummyChecksummer());
}
