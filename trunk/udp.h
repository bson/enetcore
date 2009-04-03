#ifndef __UDP_H__
#define __UDP_H__

#include "ip.h"
#include "netaddr.h"


struct Udph {
	uint16_t sport;				// Source port
	uint16_t dport;				// Dest port
	uint16_t len;				// Length
	uint16_t sum;				// Checksum

	void SetCsum() {
		sum = 0;
		sum = Htons(~ipcksum((const uint16_t*)this, sizeof (Udph) + Ntohs(len)));
	}
};


class Udp {
public:
};

#endif // __UDP_H__
