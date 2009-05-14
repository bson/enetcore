#include "enetkit.h"
#include "udp.h"
#include "ip.h"


Udp _udp0(_ip0);


UdpCoreSocket* Udp::Create()
{
	Mutex::Scoped L(_lock);
	UdpCoreSocket* s;

	// Better not have more than 32k anonymous UDP sockets... or this
	// will loop forevah. :)
	Tuple t;
	do {
		t.sport = ++_portnum;
		if (!_portnum) _portnum = 32768;
	} while (_socklist.Find(t, s));

	s = new UdpCoreSocket();
	s->_id.sport = t.sport;

	Register(s);

}


void Udp::Receive(IOBuffer* buf)
{
	Iph& iph = *(Iph*)(*buf + 0);
	Udph& udph = *(Udph*)iph.GetTransport();

	Tuple t;

	// First try finding a connected socket
	t.daddr = iph.source;
	t.sport = udph.dport;
	t.dport = udph.sport;

	UdpCoreSocket* s;

	_lock.Lock();
	if (!_socklist.Find(t, s)) {
		// Then try non-connected
		t.daddr = INADDR_ANY;
		t.dport = 0;

		if (!_socklist.Find(t, s)) {
			_lock.Unlock();
			_ip.IcmpSend(iph.source, Icmph::ICMP_DEST_UNREACH, Icmph::ICMP_PORT_UNREACH,
						 buf);
			BufferPool::FreeBuffer(buf);
			return;
		}
	}
	_lock.Unlock();

	assert(s);

	Mutex::Scoped L(s->_lock);

	s->_recvq.PushBack(buf);
	s->AddEvent(CoreSocket::EVENT_READABLE);
}


void Udp::Register(UdpCoreSocket* s)
{
	Mutex::Scoped L(_lock);
	_socklist[s->_id] = s;
}


void Udp::Deregister(UdpCoreSocket* s)
{
	Mutex::Scoped L(_lock);
	_socklist.Erase(s->_id);
}


UdpCoreSocket* Udp::Find(const Tuple& t)
{
	Mutex::Scoped L(_lock);
	UdpCoreSocket* s;
	if (_socklist.Find(t, s))  return s;

	return NULL;
}


void Udp::Checksum(IOBuffer* buf) const
{
	Iph& iph = *(Iph*)(*buf + 0);
	Udph& udph = *(Udph*)iph.GetTransport();
	udph.SetCsum(iph);
}


void Udp::IcmpError(Icmph::Type type, uint code, in_addr_t sender, in_addr_t dest,
					in_addr_t source, const Udph& udph)
{
	Tuple t;

	// First try finding a connected socket
	t.saddr = source;
	t.daddr = dest;
	t.sport = udph.sport;
	t.dport = udph.dport;

	UdpCoreSocket* s;

	Mutex::Scoped L(_lock);

	if (!_socklist.Find(t, s)) {
		// Then try non-connected
		t.daddr = INADDR_ANY;
		t.dport = 0;

		if (!_socklist.Find(t, s))
			return;
	}

	assert(s);

	s->SetError(ERR_DEST_UNREACH);
	s->AddEvent(CoreSocket::EVENT_ERROR);
}


UdpCoreSocket::~UdpCoreSocket()
{
	if (_cached_route)  _cached_route->Release();
	_udp0.Deregister(this);
}


bool UdpCoreSocket::Bind(const NetAddr& arg)
{
	Tuple t(_id);
	t.sport = Htons(arg.GetPort());
	t.saddr = arg.GetAddr4();
	if (_udp0.Find(t)) {
		CoreSocket::SetError(ERR_ADDR_IN_USE);
		return false;
	}

	// Can't hold lock across _udp calls, or we'll deadlock with
	// network thread
	_udp0.Deregister(this);
	{
		Mutex::Scoped L(_lock);
		_id = t;
	}
	_udp0.Register(this);
	return true;
}


bool UdpCoreSocket::Connect(const NetAddr& dest)
{
	_connected = true;
	Tuple t(_id);
	t.dport = Ntohs(dest.GetPort());
	t.daddr = dest.GetAddr4();
	if (_udp0.Find(t)) {
		CoreSocket::SetError(ERR_ADDR_IN_USE);
		return false;
	}

	_udp0.Deregister(this);
	{
		Mutex::Scoped L(_lock);
		_id = t;
	}
	_udp0.Register(this);
	
	return true;
}


bool UdpCoreSocket::GetSockAddr(NetAddr& addr)
{
	addr = NetAddr(_id.saddr, Ntohs(_id.sport));
}


bool UdpCoreSocket::GetPeerAddr(NetAddr& addr)
{
	addr = NetAddr(_id.daddr, Ntohs(_id.dport));
}


uint UdpCoreSocket::GetRecvAvail()
{
	if (_recvq.Empty())  return 0;

	Mutex::Scoped L(_lock);

	return _recvq.Front()->Size();
}


uint UdpCoreSocket::GetSendSpace()
{
	return 1600;
}


bool UdpCoreSocket::Send(const void* data, uint len)
{
	if (!_connected) {
		SetError(ERR_NOT_CONNECTED);
		return false;
	}

	return SendTo(data, len, NetAddr(_id.daddr, Ntohs(_id.dport)));
}


bool UdpCoreSocket::SendTo(const void* data, uint len, const NetAddr& dest)
{
	IOBuffer* buf = BufferPool::Alloc();
	if (!buf) {
		SetError(ERR_NO_SPACE);
		return false;
	}

	buf->SetHead(0);

	Iph& iph = *(Iph*)(*buf + 16);
	Udph& udph = *(Udph*)iph.GetTransport();
	udph.sport = _id.sport;
	udph.dport = Htons(dest.GetPort());
	udph.len = Htons(len + sizeof (Udph));
	buf->SetSize(16 + sizeof (Iph) + sizeof (Udph) + len);
	memcpy(udph.GetPayload(), data, len);

	iph.proto = IPPROTO_UDP;
	iph.source = INADDR_ANY /* _id.saddr */;

	Ip::Route* rt = _connected ? _cached_route : NULL;
	Ip::Route* r = _ip0.Send(buf, dest.GetAddr4(), _udp0, rt);
	if (_connected && r != _cached_route) {
		if (r) r->Retain();
		_cached_route = r;
	}

	if (!r) {
		SetError(ERR_NO_ROUTE);
		return false;
	}

	return true;
}


bool UdpCoreSocket::Recv(void* data, uint& len)
{
	NetAddr tmp;
	return RecvFrom(data, len, tmp);
}


bool UdpCoreSocket::RecvFrom(void* data, uint& len, NetAddr& sender)
{
	if (_recvq.Empty()) {
		SetError(ERR_NO_DATA);
		return false;
	}

	Mutex::Scoped L(_lock);

	IOBuffer* buf = _recvq.Front();
	_recvq.PopFront();
	buf->SetHead(16);
	Iph& iph = *(Iph*)(*buf + 0);
	Udph& udph = *(Udph*)iph.GetTransport();
	const void* payload = udph.GetPayload();
	len = min<uint>(len, Ntohs(udph.len) - sizeof (Udph));
	memcpy(data, payload, len);

	sender = NetAddr(iph.source, Ntohs(udph.sport));

	if (_recvq.Empty())
		ClearEvent(CoreSocket::EVENT_READABLE);

	BufferPool::FreeBuffer(buf);

	return true;
}


bool UdpCoreSocket::Close()
{
	_udp0.Deregister(this);
}
