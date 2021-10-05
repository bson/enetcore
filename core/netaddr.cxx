// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifdef ENABLE_IP

#include "enetkit.h"
#include "netaddr.h"


const NetAddr NetAddr::addr_any;

#ifdef POSIX
NetAddr::NetAddr(const sockaddr_storage& sa)
{
	sockaddr_in& sin = (sockaddr_in&) sa;

	if (sin.sin_family != AF_INET) {
		_ip = INADDR_ANY;
		_port = 0;
		return;
	}

	_ip = sin.sin_addr.s_addr;
	_port = sin.sin_port;
}
#endif


// * static
const NetAddr NetAddr::Parse(const String& s, bool& ok)
{
	ok = false;

	Vector<String*> parts(3);
	s.Split(parts, STR(":"));
	
	const uchar* portstr = STR("0");
	NetAddr a(addr_any);

	if (parts.Size() == 1 || parts.Size() == 2) {
		if (parts.Size() == 2) portstr = parts[1]->CStr();
	
		a = NetAddr(xinet_addr(parts[0]->CStr()), xatoi(portstr));

		ok = (a._port != (uint16_t)-1 && a._ip != (uint32_t)-1);
	}

	parts.DeleteObjects();

	return a;
}


String NetAddr::Printable(bool with_port) const
{
#ifdef POSIX
	sockaddr_storage sa = GetSAStorage();
#  if defined(__APPLE__)
	if (sa.ss_len < sizeof (sockaddr_in)) return STR("");
#  elif defined(__linux__)
	if (sa.ss_family != AF_INET && sa.ss_family != AF_INET6)  return STR("");
#  elif defined(__CYGWIN__)
	if (sa.ss_family != AF_INET)  return STR("");
#  endif

	sockaddr_in& sin = (sockaddr_in&)sa;

	uchar buf[64];
	uchar* s = xinet_ntop(sin.sin_family, &sin.sin_addr, buf, sizeof buf);
	if (with_port) {
		const uint len = xstrlen(s);
		::snprintf((char*)s + len, sizeof buf - len, ":%u", Ntohs(sin.sin_port));
	}
	return buf;
#elif defined(ENETCORE)
	const uint8_t* a = (const uint8_t*)&_ip;
	if (with_port) {
		return String::Format(STR("%u.%u.%u.%u:%u"), a[0], a[1], a[2], a[3], _port);
	} else {
		return String::Format(STR("%u.%u.%u.%u"), a[0], a[1], a[2], a[3]);
	}
#else
#error "Implement this"
#endif
}


// * static
bool NetAddr::Order(const void* a, const void* b)
{
	const NetAddr& a1 = **(const NetAddr**)a;
	const NetAddr& a2 = **(const NetAddr**)b;

	return a1._ip > a2._ip;
}


// * static
bool NetAddr::OrderWithPort(const void* a, const void* b)
{
	const NetAddr& a1 = **(const NetAddr**)a;
	const NetAddr& a2 = **(const NetAddr**)b;

	return (a1._ip > a2._ip) || (a1._ip == a2._ip && a1._port > a2._port);
}


static InterfaceInfo* GetInterface(const NetAddr& addr)
{
	const Vector<InterfaceInfo*>& iflist = GetNetworkInterfaces();

	NetAddr tmp(addr); tmp.SetPort(0);

	for (uint i = 0; i < iflist.Size(); ++i) {
		assert(iflist[i]);
		if ((iflist[i]->_addr & iflist[i]->_mask) == (tmp & iflist[i]->_mask))
			return iflist[i];
	}

	return NULL;
}


bool NetAddr::IsLocal() const
{
	InterfaceInfo* netinfo = GetInterface(*this);
	const NetAddr tmp(_ip, 0);
	return netinfo && netinfo->_addr == tmp;
}


bool NetAddr::IsLAN() const
{
	InterfaceInfo* netinfo = GetInterface(*this);
	return netinfo && !netinfo->_loopback;
}


bool NetAddr::IsLoopback() const
{
	InterfaceInfo* netinfo = GetInterface(*this);
	return netinfo && netinfo->_loopback;
}

#endif // ENABLE_IP
