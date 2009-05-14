#include "enetkit.h"
#include "dhcp.h"
#include "ip.h"
#include "udp.h"
#include "ethernet.h"
#include "util.h"
#include "dns.h"


Dhcp _dhcp0(_eth0, _dns0);

static const uint8_t dhcp_magic[] = { 0x63, 0x82, 0x53, 0x63 };

Dhcp::Dhcp(Ethernet& netif, Dns& dns) :
	_netif(netif),
	_dns(dns)
{
	_xid = Util::Random<uint32_t>();
}


void Dhcp::Reset()
{
	Mutex::Scoped L(_lock);

	_state = STATE_RESET;
	_backoff = 1;
	SendDiscover();
}


bool Dhcp::Receive(IOBuffer* buf)
{
	buf->SetHead(0);
	if (buf->Size() < _netif.GetPrealloc() + sizeof (Packet) - offsetof(Packet, options) + 5)
		return false;

	buf->SetHead(_netif.GetPrealloc());
	Packet* pkt = (Packet*)(*buf + 0);
	Iph& iph = pkt->iph;
	Udph& udp = pkt->udph;

	if (iph.proto != IPPROTO_UDP || Ntohs(udp.sport) != SERVER_PORT ||
		Ntohs(udp.dport) != CLIENT_PORT || pkt->op != BOOTREPLY) {
		BufferPool::FreeBuffer(buf);
		return false;
	}

	in_addr_t server;
	const Type msg = GetMsgType(buf, server);

	Mutex::Scoped L(_lock);
	
	if (pkt->xid == _xid) {
		switch (_state) {
		case STATE_DISCOVER:
			if (msg != DHCPOFFER)
				break;

//			DMSG("DHCP: Received DHCPOFFER");

			_server = server;
			_lease = pkt->yiaddr;
			_offer_xid = pkt->xid;
			_first_request = Time::Now();

			_backoff = 1;
			SendRequest();
			break;

		case STATE_REQUEST:
			if (msg != DHCPACK) {
				if (msg == DHCPNACK) {
					DMSG("DHCP: Received DHCPNACK, resetting");
					Reset();
				}
				break;
			}

//			DMSG("DHCP: Received DHCPACK");

			// Extract config info
			const in_addr_t prev_lease = _lease;
			const in_addr_t prev_mask = _netmask;
			const in_addr_t prev_gw = _gw;

			_lease = pkt->yiaddr;

			buf->SetHead(_netif.GetPrealloc());
			Extract(pkt->options, buf->Size() - offsetof(Packet, options));

			_state = STATE_CONFIGURED;
			_backoff = 0;

			const NetAddr addr(_lease, 0);
			const NetAddr mask(_netmask, 0);
			const NetAddr gw(_gw, 0);
			const NetAddr dns(_ns, 0);
			const uint expire = (_renew - Time::Now()).GetSec();
			console("DHCP: addr %a/%a  gw %a", &addr, &mask, &gw);
			console("DHCP: name server %a, domain \"%S\"", &dns, &_domain);
			DMSG("DHCP: ttl %u sec", expire);

			if (_lease != prev_lease || _netmask != prev_mask || _gw != prev_gw) {
				_ip.RemoveInterface(_netif);
				_ip.AddInterface(_netif, _lease, _netmask);
				_ip.AddDefaultRoute(_netif, _gw);
			}

			_dns.SetDomain(_domain);
			_dns.SetNS(_ns);

			break;
		}
	}

	BufferPool::FreeBuffer(buf);
	return true;
}


Time Dhcp::GetServiceTime()
{
	Mutex::Scoped L(_lock);

	switch (_state) {
	case STATE_RESET:
	case STATE_DISCOVER:
	case STATE_REQUEST:
		return _rexmit;
	case STATE_CONFIGURED:
		return _renew;
	}

	return Time::Now() + Time::FromSec(120);
}


Time Dhcp::Service()
{
	Mutex::Scoped L(_lock);

	switch (_state) {
	case STATE_CONFIGURED:
		if (Time::Now() < _renew)
			break;

		// Request extension
		_state = STATE_REQUEST;
		_backoff = 1;

		// fallthrough
	case STATE_REQUEST:
		if (_backoff < 128) {
			SendRequest();
			break;
		}

		// Start over after four retransmits
		_backoff = 1;
		_state = STATE_DISCOVER;
		// fallthrough

	case STATE_RESET:
		if (_state == STATE_RESET && Time::Now() < _rexmit)
			break;

	case STATE_DISCOVER:
	case_STATE_DISCOVER:
		if (_backoff == 128)  _backoff = 1;
		SendDiscover();
		break;
	}

	return GetServiceTime();
}


void Dhcp::LinkRecovered()
{
	if (_state == STATE_CONFIGURED) _renew = Time::Now();
}


IOBuffer* Dhcp::AllocPacket()
{
	IOBuffer* buf = BufferPool::Alloc();
	if (!buf) return NULL;

	buf->SetHead(_netif.GetPrealloc());
	buf->SetSize(sizeof (Packet));

	// Initialize packet with defaults
	Packet* pkt = (Packet*)(*buf + 0);

	memset(pkt, 0, offsetof(Packet, options));

	pkt->op = BOOTREQUEST;
	pkt->htype = _netif.GetBootpType();
	pkt->hlen = _netif.GetAddrLen();
	pkt->xid = _xid;			// Default XID
	pkt->secs = (Time::Now() - _start).GetSec();
//	pkt->flags = Htons(1);		// Tell server to broadcast reply
	memcpy(pkt->chaddr, _netif.GetMacAddr(), _netif.GetAddrLen());

	return buf;
}


void Dhcp::SendDiscover()
{
	_lock.AssertLocked();

	IOBuffer* buf = AllocPacket();
	if (!buf) {
		DMSG("DHCP: SendDiscover: no buffers");
		_rexmit = Time::Now() + Time::FromSec(1);
		return;
	}
		
	if (_state == STATE_RESET)
		_start = Time::Now();

	_state = STATE_DISCOVER;

	static const uint8_t request[] = {
		0x63, 0x82, 0x53, 0x63,
		TAG_DHCP_MSGTYPE, 1, DHCPDISCOVER,
		TAG_DHCP_PARAM_REQ, 4, TAG_SUBNET, TAG_GW, TAG_NS, TAG_DOMAIN,
		TAG_DHCP_MAX_SIZE, 2, 0x05, 0xdc,
		TAG_END
	};

	Packet* pkt = (Packet*)(*buf + 0);
	memcpy(pkt->options, request, sizeof request);
	buf->SetSize(offsetof(Packet, options) + sizeof request);

	FillHeader(buf);

	// Hand over packet to MAC
//	DMSG("DHCP: broadcasting DHCPDISCOVER");
	_netif.Send(buf);

	BackOff();					// Update rexmit timer
}


void Dhcp::SendRequest()
{
	_lock.AssertLocked();

	static const uint8_t request[] = {
		0x63, 0x82, 0x53, 0x63,
		TAG_DHCP_MSGTYPE, 1, DHCPREQUEST,
		TAG_DHCP_SERVER, 4, 0, 0, 0, 0,
		TAG_DHCP_REQ_IP, 4, 0, 0, 0, 0,
		TAG_DHCP_PARAM_REQ, 4, TAG_SUBNET, TAG_GW, TAG_NS, TAG_DOMAIN,
		TAG_DHCP_MAX_SIZE, 2, 0x05, 0xdc,
		TAG_END
	};

	IOBuffer* buf = AllocPacket();
	if (!buf) {
		DMSG("DHCP: SendRequest: no buffers");
		_rexmit = Time::Now() + Time::FromSec(1);
		return;
	}

	_state = STATE_REQUEST;

	Packet* pkt = (Packet*)(*buf + 0);
	pkt->xid = _offer_xid;
	
	memcpy(pkt->options, request, sizeof request);
	memcpy(pkt->options + 9, &_server, 4);
	memcpy(pkt->options + 15, &_lease, 4);

	buf->SetSize(offsetof(Packet, options) + sizeof request);

	FillHeader(buf);

	// Hand over packet to ethernet
//	DMSG("DHCP: broadcasting DHCPREQUEST");
	_netif.Send(buf);

	BackOff();					// Update rexmit timer
}


void Dhcp::BackOff()
{
	_lock.AssertLocked();

	// XXX should be 1,9,13,16,300 sec?

	_backoff *= 2;
	_rexmit = Time::Now() + Time::FromSec(_backoff);
}


void Dhcp::Extract(const uint8_t* options, uint len)
{
	if (len < 5 || memcmp(options, dhcp_magic, 4))
		return;

	_netmask = INADDR_ANY;
	_gw = INADDR_ANY;
	_ns = INADDR_ANY;
	_domain = STR("");

	for (const uint8_t* p = options + 4; p < options + len; ) {
		const Tag tag = (Tag)*p++;
		if (tag == TAG_PAD) continue;

		const uint len = *p++;

		switch (tag) {
		case TAG_SUBNET:
			memcpy(&_netmask, p, 4);
			break;
		case TAG_GW: {
			const uint numgw = len / 4;
			memcpy(&_gw, p, 4);
			break;
		}
		case TAG_NS: {
			const uint numns = len / 4;
			memcpy(&_ns, p, 4);
			break;
		}
		case TAG_DOMAIN: {
			_domain = String(p, len);
			break;
		}
		case TAG_END:
			p = options + len;
			break;
		case TAG_DHCP_LEASE: {
			uint32_t secs;
			memcpy(&secs, p, 4);
			_renew = _first_request + Time::FromSec(Ntohl(secs));
			break;
		}
		}
		p += len;
	}
done:
	if (_netmask == INADDR_ANY)
		_netmask = 0xffffff00;
}


Dhcp::Type Dhcp::GetMsgType(IOBuffer* buf, in_addr_t& server)
{
	const Packet* pkt = (Packet*)(*buf + 0);
	const uint8_t* options = pkt->options;
	const uint len = buf->Size() - offsetof(Packet, options);

	server = INADDR_ANY;
	Type type = DHCPINVALID;

	if (len < 5 || memcmp(options, dhcp_magic, 4))
		return type;

	bool got_server = false;
	bool got_type = false;

	for (const uint8_t* p = options + 4; p < options + len; ) {
		const Tag tag = (Tag)*p++;
		if (tag == TAG_PAD) continue;

		const uint len = *p++;

		switch (tag) {
		case TAG_DHCP_SERVER:
			memcpy(&server, p, 4);
			got_server = true;
			break;
		case TAG_DHCP_MSGTYPE:
			type = (Type)*p;
			got_type = true;
			break;
		case TAG_END:
			p = options + len;
			break;
		}
		if (got_server && got_type) break;

		p += len;
	}
	return type;
}


void Dhcp::FillHeader(IOBuffer* buf)
{
	// Clearing out the headers makes it easier to diagnose uninitialized fields
//	memset(*buf + 0 - _netif.GetPrealloc(), 0x55, offsetof(Packet, op) + _netif.GetPrealloc());

	Packet* pkt = (Packet*)(*buf + 0);
	Iph& iph = pkt->iph;
	iph.SetHLen(sizeof (Iph));
	iph.tos = 0;
	iph.len = Htons(buf->Size());
	iph.id = _ip.GetId();
	iph.off = 0;
	iph.ttl = 255;
	iph.proto = IPPROTO_UDP;
	iph.source = INADDR_ANY;
	iph.dest = ~0;
	iph.SetCsum();

	Udph& udph = pkt->udph;
	udph.sport = Htons(CLIENT_PORT);
	udph.dport = Htons(SERVER_PORT);
	udph.len = Htons(buf->Size() - sizeof (Iph));
	udph.SetCsum(iph);

	udph.sum = 0;

	buf->SetHead(0);
	_netif.FillForBcast(buf, ETHERTYPE_IP);
}
