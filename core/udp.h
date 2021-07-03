// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __UDP_H__
#define __UDP_H__

#include "ip.h"
#include "netaddr.h"
#include "ipconn.h"
#include "socket.h"
#include "hashmap.h"


struct __novtable Udph {
	uint16_t sport;				// Source port
	uint16_t dport;				// Dest port
	uint16_t len;				// Length
	mutable uint16_t sum;		// Checksum

	uint16_t CSum(const Iph& iph) const {
		const uint16_t tmp = exch<uint16_t>(sum, 0);
		const uint16_t csum = ~ipcksum((const uint16_t*)this, Ntohs(len), iph.SumPH());
		sum = tmp;
		return csum ? Htons(csum) : ~0;
	}

	void SetCsum(const Iph& iph) { sum = CSum(iph); }

	bool ValidateCsum(const Iph& iph) const {
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


class Udp: public Checksummer {
	friend class UdpCoreSocket;

	Mutex _lock;
	Ip& _ip;

	HashMap<Tuple, UdpCoreSocket*> _socklist;

	uint16_t _portnum;

public:
	Udp(Ip& ip) : _ip(ip), _portnum(32768) { }

	// Create UDP socket
	UdpCoreSocket* Create();
	void Receive(IOBuffer* buf);

	void Checksum(IOBuffer* buf) const; // * implements Checksummer::Checksum

	void IcmpError(Icmph::Type type, uint code,
				   in_addr_t sender, // Sender of ICMP message (e.g., router)
				   in_addr_t dest,	 // Destination datagram was for
				   in_addr_t source, // Datagram sender (i.e., ourselves)
				   const Udph& udp); // Start of original packet

protected:
	void Deregister(UdpCoreSocket* s);
	void Register(UdpCoreSocket* s);
	UdpCoreSocket* Find(const Tuple& t);
};


extern Udp _udp0;


#endif // __UDP_H__
