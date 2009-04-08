#include "enetkit.h"
#include "dhcp.h"
#include "ip.h"
#include "udp.h"
#include "ethernet.h"
#include "util.h"


Dhcp _dhcp0(_eth0);


Dhcp::Dhcp(Ethernet& netif) : _netif(netif)
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
	Type msg = GetMsgType(buf, server);

	Mutex::Scoped L(_lock);
	
	if (pkt->xid == _xid) {
		switch (_state) {
		case STATE_DISCOVER:
			if (msg != DHCPOFFER)
				break;

			DMSG("DHCP: Received DHCPOFFER");

			_server = server;
			_lease = pkt->yiaddr;
			_offer_xid = pkt->xid;
			_first_request = Time::Now();

			_backoff = 1;
			SendRequest();
			break;

		case STATE_REQUEST:
			if (msg != DHCPACK)
				break;

			_lease = pkt->yiaddr;

			// Extract config info
			buf->SetHead(pkt->options - (*buf + 0));
			Extract(pkt->options, buf->Size());

			_state = STATE_CONFIGURED;
			_backoff = 0;

			const NetAddr addr(_lease);
			const NetAddr mask(_netmask);
			const NetAddr gw(_gw);
			const NetAddr dns(_dns);
			const uint expire = (_renew - Time::Now()).GetSec();
			console("DHCP: addr %a/%a  gw %a", &addr, &mask, &gw);
			console("DHCP: name server %a, domain \"%S\"", &dns, &_domain);
			DMSG("DHCP: ttl %u sec", expire);

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

	memset(&pkt->op, 0, offsetof(Packet, options));

	pkt->op = BOOTREQUEST;
	pkt->htype = _netif.GetBootpType();
	pkt->hlen = _netif.GetAddrLen();
	pkt->xid = _xid;			// Default XID
	pkt->secs = (Time::Now() - _start).GetSec();
//	pkt->flags = 1;			// Tell server to broadcast reply
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
		9, 130, 83, 99,			// DHCP magic
		TAG_VEXT, 10,
 		  TAG_DHCP_MSGTYPE, 1, DHCPDISCOVER,
		  TAG_DHCP_PARAM_REQ, 4, TAG_SUBNET, TAG_GW, TAG_NS, TAG_DOMAIN,
		  TAG_END,
		TAG_END
	};

	Packet* pkt = (Packet*)(*buf + 0);
	memcpy(pkt->options, request, sizeof request);
	buf->SetSize(offsetof(Packet, options) + sizeof request);

	FillHeader(buf);

	// Hand over packet to MAC
	DMSG("DHCP: broadcasting DHCPDISCOVER");
	_netif.Send(buf);

	BackOff();					// Update rexmit timer
}


void Dhcp::SendRequest()
{
	_lock.AssertLocked();

	static const uint8_t request[] = {
		9, 130, 83, 99,			// DHCP magic
		TAG_VEXT, 16,
 		  TAG_DHCP_MSGTYPE, 1, DHCPREQUEST,
		  TAG_DHCP_SERVER, 4, 0, 0, 0, 0,
		  TAG_DHCP_PARAM_REQ, 4, TAG_SUBNET, TAG_GW, TAG_NS, TAG_DOMAIN,
		  TAG_END,
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
	memcpy(pkt->options + 11, &_server, 4);
	buf->SetSize(offsetof(Packet, options) + sizeof request);

	FillHeader(buf);

	// Hand over packet to ethernet
	DMSG("DHCP: broadcasting DHCPREQUEST");
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
	_netmask = INADDR_ANY;
	_gw = INADDR_ANY;
	_dns = INADDR_ANY;
	_domain = STR("");

	for (const uint8_t* p = options; p < options + len; ) {
		switch (*p++) {
		case TAG_PAD: break;
		case TAG_SUBNET:
			// RFC1497 says this is a fixed-length field
			// RFC1533 says it has a length prologue that's always 4
			// If we see a '4' here we assume it's RFC1533 compliant
			if (*p == 4) ++p;
			memcpy(&_netmask, p, 4);
			p += 4;
			break;
		case TAG_GW: {
			const uint numgw = *p++;
			memcpy(&_gw, p, 4);
			p += numgw;
			break;
		}
		case TAG_NS: {
			const uint numns = *p++;
			memcpy(&_dns, p, 4);
			p += numns;
			break;
		}
		case TAG_DOMAIN: {
			const uint len = *p++;
			_domain = String(p, len);
			p += len;
			break;
		}
		case TAG_END:
			p = options + len;
			break;
		case TAG_VEXT: {
			const uint len = *p++;
			for (const uint8_t* v = p; v < p + len; ++v) {
				switch (*v++) {
				case TAG_PAD: break;
				case TAG_END: v = p + len; break;
				case TAG_DHCP_LEASE: {
					uint32_t secs;
					memcpy(&secs, ++v, 4);
					v += 4;
					_renew = _first_request + Time::FromSec(secs);
					break;
				}
				default:
					v += *v + 1;
					break;
				}
			}
			p += len;
			break;
		}
		default:
			// Some tag we don't care about
			p += *p + 1;
			break;
		}
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

	Type type = DHCPINVALID;
	server = INADDR_ANY;
	bool got_server = false;
	bool got_type = false;

	for (const uint8_t* p = options; p < options + len; ++p) {
		switch (*p++) {
		case TAG_PAD: break;
		case TAG_END: goto done;
		case TAG_SUBNET:
			if (*p == 4) ++p;
			p += 4;
			break;
		case TAG_VEXT: {
			const uint vlen = *p++;
			for (const uint8_t* v = p; v < p + vlen; ++v) {
				switch (*v++) {
				case TAG_DHCP_SERVER:
					memcpy(&server, ++v, 4);
					v += 4;
					got_server = true;
					break;
				case TAG_DHCP_MSGTYPE:
					type = (Type)*v++;
					got_server = true;
					break;
				case TAG_END:
					v = p + vlen;
					break;
				}
				if (got_server && got_type) return type;
			}
			p += vlen;
			break;
		}
		default:
			p += *p + 1;
			break;
		}
	}
done:
	return type;
}


void Dhcp::FillHeader(IOBuffer* buf)
{
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

	buf->SetHead(0);
	_netif.FillForBcast(buf, ETHERTYPE_IP);
}
