#ifndef __UDP_H__
#define __UDP_H__

#include "netaddr.h"


struct Udph {
	uint16_t sport;				// Source port
	uint16_t dport;				// Dest port
	uint16_t len;				// Length
	uint16_t csum;				// Checksum
};


class Udp {
public:
	// Fill in packet from scratch - UDP, IP, MAC
	static void FillHeader(IOBuffer* buf, const NetAddr& src, const NetAddr& dst);
};

#endif // __UDP_H__
