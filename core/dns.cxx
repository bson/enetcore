// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifdef ENABLE_IP

#include "core/enetkit.h"
#include "core/dns.h"
#include "core/udp.h"

Dns _dns0(_udp0);


Dns::Dns(Udp& udp) :
	_udp(udp),
	_id(0),
	_sock(NULL),
	_state(STATE_IDLE)
{
}


void Dns::Init()
{
	Mutex::Scoped L(_lock);

	_sock = _udp0.Create();
	assert(_sock);
	_sock->SetEventMask(CoreSocket::EVENT_READABLE);
	_sock->Bind(NetAddr(INADDR_ANY, DNS_PORT));
}


void Dns::SetNS(in_addr_t ns)
{
	Mutex::Scoped L(_lock);

	if (ns != _ns) {
		_ns = ns;

		NetAddr a(ns, DNS_PORT);
		DMSG("DNS: NS set to %A", &a);

		assert(_sock);
		_sock->Connect(NetAddr(ns, DNS_PORT));
		_change.Signal();
	}
}


bool Dns::GetAddrByName(const String& name, in_addr_t& addr)
{
	Acquire();

	assert(_sock);

	for (uint attempt = 0; attempt < MAX_ATTEMPTS; ++attempt) {
		Deque<uchar> req;

		DMSG("DNS: IN A query for %S", &name);

		// Generate fresh IN A query
		Dnsh::CreateLookupQuery(req, name);

		const Time deadline = Time::Now() + Time::FromSec(NS_TIMEOUT);

		_sock->Send(req + 0, req.Size());
		while (Time::Now() < deadline) {
			if (!_sock->Wait(deadline - Time::Now())) break;

			// Drain replies, reuse req
			for (;;) {
				req.Clear();
				uint len = 512;
				if (!_sock->Recv(req + req.Grow(len), len)) break;

				uint rcode;
				if (Dnsh::GetRRA1(req, addr, rcode)) {
					Release();
					NetAddr a(addr, 0);
					DMSG("DNS: %S: result: %a", &name, &a);
					return true;
				}
				DMSG("DNS: IN A query for %S failed: rcode=%u", &name, rcode);
			}
		}
	}

	Release();
	return false;
}


void Dns::Acquire()
{
	Mutex::Scoped L(_lock);
	while (_state != STATE_IDLE)
		_change.Wait(_lock);
	_state = STATE_RESOLVING;
}


void Dns::SetState(State new_state)
{
	Mutex::Scoped L(_lock);
	_state = new_state;
	_change.Signal();

}

	
void Dns::Release() { SetState(STATE_IDLE); }


// * static
void Dnsh::CreateLookupQuery(Deque<uint8_t>& buf, const String& host)
{
	String fqdn(host);

	if (fqdn.FindFirst('.') == NOT_FOUND) {
		fqdn += STR(".");
		fqdn += _dns0.GetDomain();
	}

	Dnsh& dnsh = *(Dnsh*)(buf + buf.Grow(sizeof (Dnsh)));

	memset(&dnsh, 0, sizeof dnsh);

	// dnsh.opcode = OPCODE_QUERY;

	dnsh.id = Htons(_dns0.NextId());
	dnsh.rd = true;
	dnsh.qr = true;
	dnsh.questions = Htons(1);

	const uint query_len = 4 + fqdn.Size() + 1;

	uint8_t* query = buf + buf.Grow(query_len);

	uint pos = 0;				// Current char in fqdn
	uint part_pos = 0;			// First char in fqdn part
	uint part_query = 0;		// Current char in query part
	uint query_pos = 0;			// Current pos in query

	const uint host_len = fqdn.Size();

	while (pos < host_len) {
		if (fqdn[pos] == '.')  {
			const uint part_len = pos - part_pos;
			query[part_query] = part_len;
			part_query += part_len + 1;
			part_pos = ++pos;
			++query_pos;
		} else {
			query[++query_pos] = fqdn[pos++];
		}
	}

	// 0 for the root (single dot)
	query[query_len-1] = 0;

	const uint16_t type_class[2] = { Htons(TYPE_A), Htons(CLASS_IN) };

	memcpy(buf + buf.Grow(4), type_class, 4);
}


// * static
bool Dnsh::GetRRA1(const Deque<uint8_t>& dnspkt, in_addr_t& addr, uint& rcode)
{
	rcode = 0;

	if (dnspkt.Size() < sizeof (Dnsh) + 4 + 10) return false;

	const Dnsh& dnsh = *(const Dnsh*)&dnspkt.Front();

	rcode = dnsh.rcode;

	if (Ntohs(dnsh.id) != _dns0.CurId() || dnsh.qr || dnsh.rcode || !dnsh.questions ||
		!dnsh.rr_answers)
		return false;

	uint questions = 0;
	// Skip past any question up front
	// XXX we should really check it's OUR question...
	for (uint q = 0; q < dnsh.questions && questions < dnspkt.Size(); ++q){
		while (questions < dnspkt.Size() && dnspkt[questions])
			++questions;

		if (dnspkt.Size() - questions < 4)
			return false;

		questions += 4;
	}

	if (dnspkt.Size() < sizeof (Dnsh) + questions + 1)
		return false;

	const uint8_t* rr = dnspkt + sizeof (Dnsh) + questions; 

	uint len = &dnspkt.Back() - rr;
	
	while (len-- && *rr++) continue;

	if (len < sizeof (DnsRdata)) return false;

	DnsRdata rd;
	memcpy(&rd, rr, sizeof (DnsRdata));
	rr += sizeof (DnsRdata);
	len -= sizeof (DnsRdata);
	
	rd.cl = Ntohs(rd.cl);
	rd.type = Ntohs(rd.type);
	rd.length = Ntohs(rd.length);
//	rd.ttl = Ntohl(rd.ttl);

	if (rd.type != TYPE_A || rd.cl != CLASS_IN || rd.length != 4 ||
		len < 4)
		return false;
		
	in_addr_t tmp;
	memcpy(&tmp, rr, 4);
	addr = Ntohl(tmp);

	return true;
}

#endif // ENABLE_IP
