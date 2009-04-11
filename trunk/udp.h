#ifndef __UDP_H__
#define __UDP_H__

#include "ip.h"
#include "netaddr.h"
#include "ipconn.h"
#include "socket.h"
#include "hashmap.h"


struct NOVTABLE Udph {
	uint16_t sport;				// Source port
	uint16_t dport;				// Dest port
	uint16_t len;				// Length
	uint16_t sum;				// Checksum

	uint16_t CSum(const Iph& iph) {
		sum = 0;
		uint16_t csum = ipcksum((const uint16_t*)this, Ntohs(len),
								iph.source + iph.dest + iph.proto + iph.len);
		csum = ~Htons(csum);
		if (!csum) --csum;
		return csum;
	}

	void SetCsum(const Iph& iph) { sum = CSum(iph); }

	bool ValidateCsum(const Iph& iph) {
		if (!sum) return true;

		const int16_t tmp = exch<uint16_t>(sum, 0);
		const bool equal = tmp == CSum(iph);
		sum = tmp;
		return equal;
	}

	uint8_t* GetPayload() { return (uint8_t*)this + sizeof (Udph); }
};


class UdpCoreSocket: public CoreSocket {
	friend class Udp;
protected:
	Mutex _lock;

	// Socket identifier
	Tuple _id;

	// For a connected socket, the last route
	Ip::Route* _cached_route;

	// Receive queue
	Deque<IOBuffer*> _recvq;

	// Flags
	bool _connected:1;			// Connected UDP socket

public:
	~UdpCoreSocket();

	bool Bind(const NetAddr& arg);
	bool Connect(const NetAddr& dest);
	bool GetSockAddr(NetAddr& addr);
	bool GetPeerAddr(NetAddr& addr);
	uint GetRecvAvail();
	uint GetSendSpace();
	bool Send(const void* data, uint len);
	bool SendTo(const void* data, uint len, const NetAddr& dest);
	bool Recv(void* data, uint& len);
	bool RecvFrom(void* data, uint& len, NetAddr& sender);
	bool Close();
};


class Udp {
	friend class UdpCoreSocket;

	Mutex _lock;
	HashMap<Tuple, UdpCoreSocket*> _socklist;

	uint16_t _portnum;

public:
	Udp() : _portnum(32768) { }

	// Create UDP socket
	UdpCoreSocket* Create();
	void Receive(IOBuffer* buf);

protected:
	void Deregister(UdpCoreSocket* s);
	void Register(UdpCoreSocket* s);
	UdpCoreSocket* Find(const Tuple& t);
};


extern Udp _udp;


#endif // __UDP_H__
