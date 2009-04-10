#include "enetkit.h"
#include "udp.h"


Udp _udp;


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
	buf->SetHead(16);
	Iph& iph = *(Iph*)(*buf + 0);
	Udph& udph = *(Udph*)iph.GetTransport();

	Tuple t;

	// First try finding a connected socket
	t.daddr = iph.source;
	t.sport = udph.dport;
	t.dport = udph.sport;

	UdpCoreSocket* s;

	{
		Mutex::Scoped L(_lock);
		if (!_socklist.Find(t, s)) {
			// Then try non-connected
			t.daddr = INADDR_ANY;
			t.dport = 0;

			if (!_socklist.Find(t, s)) {
				// XXX
				// IcmpError(... ICMP_PORT_UNREACH ...)
				BufferPool::FreeBuffer(buf);
				return;
			}
		}
	}

	Mutex::Scoped L2(s->_lock);

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


UdpCoreSocket::~UdpCoreSocket()
{
	_udp.Deregister(this);
}


bool UdpCoreSocket::Bind(const NetAddr& arg)
{
	Tuple t(_id);
	t.sport = Htons(arg.GetPort());
	t.saddr = arg.GetAddr4();
	if (_udp.Find(t)) {
		CoreSocket::SetError(ERR_ADDR_IN_USE);
		return false;
	}

	// Can't hold lock across _udp calls, or we'll deadlock with
	// network thread
	_udp.Deregister(this);
	{
		Mutex::Scoped L(_lock);
		_id = t;
	}
	_udp.Register(this);
	return true;
}


bool UdpCoreSocket::Connect(const NetAddr& dest)
{
	_connected = true;
	Tuple t(_id);
	t.dport = Ntohs(dest.GetPort());
	t.daddr = dest.GetAddr4();
	if (_udp.Find(t)) {
		CoreSocket::SetError(ERR_ADDR_IN_USE);
		return false;
	}

	_udp.Deregister(this);
	{
		Mutex::Scoped L(_lock);
		_id = t;
	}
	_udp.Register(this);
	
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
}


bool UdpCoreSocket::SendTo(const void* data, uint len, const NetAddr& dest)
{
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
	_udp.Deregister(this);
}
