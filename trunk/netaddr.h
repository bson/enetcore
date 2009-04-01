#ifndef __NETADDR_H__
#define __NETADDR_H__

#include "pstring.h"
#include "arithmetic.h"

enum { INADDR_ANY = 0 };

// Currently only supports IPv4
// Addr, port are in network byte order throughout

class NetAddr {
	in_addr_t _ip;
	uint16_t _port;
public:
	NetAddr(in_addr_t ip, uint16_t port) : _ip(ip), _port(Htons(port)) { }
	NetAddr(uint16_t port = 0) : _ip(INADDR_ANY), _port(Htons(port)) { }
	NetAddr(const NetAddr& arg) { assign(arg); }
	virtual ~NetAddr() { }

	NetAddr& assign(const NetAddr& arg) {
		if (&arg != this) {
			_ip = arg._ip;
			_port = arg._port;
		}
		return *this;
	}

	NetAddr& operator=(const NetAddr& arg) { return assign(arg); }
	
#ifdef POSIX
	NetAddr(const sockaddr_storage& sa);

	// Obtain SA for bind(), connect(), etc
	// This is inlined to facilitate in-place construction
	const sockaddr_storage GetSAStorage() const
	{
		struct sockaddr_storage sa;
		struct sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(&sa);

		memset(sin, 0, sizeof *sin);
#ifdef __APPLE__
		sin->sin_len = sizeof *sin;
#endif
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = _ip;
		sin->sin_port = _port;

		return sa;
	}
#endif

	in_addr_t GetAddr4() const { return _ip; }
	uint16_t GetPort() const { return Ntohs(_port); }

	void SetAddr4(in_addr_t arg) { _ip = arg; }
	void SetPort(uint16_t arg) { _port = Htons(arg); }

	bool IsAddrAny() const { return _ip == INADDR_ANY; }

	static const NetAddr Parse(String s, bool& ok);

	String Printable(bool with_port = true) const;

	bool operator==(const NetAddr& arg) const { return _ip == arg._ip && _port == arg._port; }
	bool operator!=(const NetAddr& arg) const { return _ip != arg._ip || _port != arg._port; }

	// For ordering based on IP addr, true if *a > *b
	static bool Order(const void* a, const void* b);

	// For ordering based on IP addr+port
	static bool OrderWithPort(const void* a, const void* b);

	static const NetAddr addr_any;
};

#endif // __NETADDR_H__
