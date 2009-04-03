#include "enetkit.h"
#include "ip.h"


Ip _ip;


void Ip::AddInterface(Ethernet& nic, in_addr_t addr, in_addr_t mask) 
{
	Mutex::Scoped L(_lock);

	RemoveInterface(nic);
	_routes.PushFront(new Route(nic, Route::TYPE_IF, addr, mask));
}


void Ip::RemoveInterface(Ethernet& nic) 
{
	Mutex::Scoped L(_lock);

	for (uint i = 0; i < _routes.Size(); ) {
		if (&(_routes[i]->netif) == &nic) {
			delete _routes[i];
			_routes.Erase(i);
		} else {
			++i;
		}
	}

	// Prune pending ARP
	ReleasePendingARP();
}


void Ip::AddDefaultRoute(Ethernet& nic, in_addr_t router)
{
	Mutex::Scoped L(_lock);

	// Insert immediately following last TYPE_IF
	uint pos;
	for (pos = _routes.Size() - 1; pos >= 0; --pos) {
		if (_routes[pos]->type == Route::TYPE_IF) {
			Route* rt = new Route(nic, Route::TYPE_RT, 0, 0);
			rt->nexthop = router;
			_routes.Insert(pos + 1) = rt;
			break;
		}
	}
}


void Ip::AddNetRoute(Ethernet& nic, in_addr_t network, in_addr_t netmask, in_addr_t router)
{
	Mutex::Scoped L(_lock);

	// Insert immediately following last TYPE_RT
	for (uint pos = _routes.Size() - 1; pos >= 0; --pos) {
		if (_routes[pos]->type == Route::TYPE_RT) {
			Route* rt = new Route(nic, Route::TYPE_RT, network, netmask);
			rt->nexthop = router;
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
	_routes.PushBack(hostrt);

	return hostrt;
}


void Ip::ReleasePendingARP()
{
	Mutex::Scoped L(_lock);

	for (uint i = 0; i < _pending_arp.Size(); ) {
		IOBuffer* packet = _pending_arp[i];
		const in_addr_t dest = GetIph(packet).dest;
		bool remove_packet = false;

		for (uint n = _routes.Size() - 1; n >= 0; --n) {
			Route* rt = _routes[n];
			if (rt->dest == dest & rt->netmask) {
				if (rt->macvalid) {
					memcpy(*packet + 2, rt->macaddr, 6);
					rt->netif.Send(packet);
					remove_packet = true;
				}
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


bool Ip::Send(IOBuffer* buf)
{
	Mutex::Scoped L(_lock);
	const in_addr_t dest = GetIph(buf).dest;

	for (uint i = _routes.Size()-1; i >= 0; --i) {
		Route* rt = _routes[i];
		if (rt->dest == dest & rt->netmask) {
			if (rt->macvalid) {
				memcpy(*buf + 2, rt->macaddr, 6);
				if (rt->type == Route::TYPE_IF) {
					if (rt->dest == dest) {
						rt->netif.Send(buf); // Loopback: ethernet driver will return it
						return true;
					} else {
						Route* hostrt = AddHostRoute(dest, rt);
						_pending_arp.PushBack(buf);
						RequestARP(hostrt);
						return false;
					}
				} else {
					rt->netif.Send(buf);
					return true;
				}
			} else {
				_pending_arp.PushBack(buf);
				RequestARP(rt);
				return false;
			}
		}
	}

	BufferPool::FreeBuffer(buf);
	// XXX set socket error
	return true;
}


void Ip::Receive(IOBuffer* packet)
{
	Mutex::Scoped L(_lock);
	const in_addr_t dest = GetIph(packet).dest;
	const in_addr_t source = GetIph(packet).source;
	Route* hostrt = NULL;
	Route* netif = NULL;

	for (uint i = _routes.Size()-1; i >= 0; --i) {
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
		// IcmpErrorReply(packet, ICMP_UNREACH, ICMP_HOST_UNREACH);
		BufferPool::FreeBuffer(packet);
		return;
	}

	if (!hostrt) {
		hostrt = AddHostRoute(source, netif);
		memcpy(hostrt->macaddr, GetMacSource(packet), 6);
		hostrt->macvalid = true;
		ReleasePendingARP();
	}

	const uint8_t proto = GetIph(packet).proto;
	switch (proto) {
#if 0
	case IPPROTO_ICMP:
		IcmpReceive(packet);
		break;
#endif
#if 0
	case IPPROTO_UDP:
		Udp::Receive(packet);
		break;
#endif
#if 0
	case IPPROTO_TCP:
		Tcp::Receive(packet);
		break;
#endif
	default:
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

	for (uint i = _routes.Size()-1; i >= 0; --i) {
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
