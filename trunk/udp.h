#ifndef __UDP_H__
#define __UDP_H__

#include "ip.h"
#include "netaddr.h"


struct Udph {
	uint16_t sport;				// Source port
	uint16_t dport;				// Dest port
	uint16_t len;				// Length
	uint16_t sum;				// Checksum

	// UDP pseudo header
	struct UPH {
		in_addr_t saddr;
		in_addr_t daddr;
		uint8_t zero;
		uint8_t proto;
		uint16_t len;
	};

	uint16_t CSum(const Iph& iph) const {
		UPH ph = { iph.source, iph.dest, 0, iph.proto, len };

		uint16_t csum = ipcksum((const uint16_t*)&ph, sizeof ph);
		csum = ipcksum((const uint16_t*)(uint8_t*)this + sizeof (Udph),
					   Ntohs(len) - sizeof (Udph), csum);
		csum = Htons(~csum);
		if (!csum) --csum;
		return csum;
	}

	void SetCsum(const Iph& iph) { sum = CSum(iph); }
	bool ValidateCsum(const Iph& iph) const { return sum == CSum(iph); }

	uint8_t* GetPayload() { return (uint8_t*)this + sizeof (Udph); }
};


class Udp {
public:
};

#endif // __UDP_H__
