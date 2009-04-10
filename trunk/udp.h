#ifndef __UDP_H__
#define __UDP_H__

#include "ip.h"
#include "netaddr.h"
#include "ipconn.h"
#include "socket.h"


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
		const int16_t tmp = exch<uint16_t>(sum, 0);
		const bool equal = tmp == CSum(iph);
		sum = tmp;
		return equal;
	}

	uint8_t* GetPayload() { return (uint8_t*)this + sizeof (Udph); }
};


class UdpCoreSocket: public CoreSocket {
};


class Udp {
public:
	// Create UDP socket
	static UdpCoreSocket* Create();
};

#endif // __UDP_H__
