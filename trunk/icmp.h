#ifndef __ICMP_H__
#define __ICMP_H__


struct __novtable Icmph {
	// ICMP message types
	enum Type {
		ICMP_ECHO_REPLY, ICMP_1, ICMP_2, ICMP_DEST_UNREACH,
		ICMP_SRC_QUENCH, ICMP_REDIRECT, ICMP_6, ICMP_7,
		ICMP_ECHO_REQ, ICMP_RTR_ADVERT, ICMP_RTR_SOLICIT,
		ICMP_TTL_EXC, ICMP_PARAM_ERR, ICMP_TIME_REQ,
		ICMP_TIME_REPLY, ICMP_15, ICMP_16, ICMP_MASK_REQ,
		ICMP_MASK_REPLY, ICMP_MAX_TYPE
	};

	// Dest unreach codes
	enum DestUnreach {
		ICMP_NET_UNREACH, ICMP_HOST_UNREACH, ICMP_PROTO_UNREACH, ICMP_PORT_UNREACH,
		ICMP_DF_SET, ICMP_SR_ERR, ICMP_NET_ERR, ICMP_HOST_ERR, ICMP_8,
		ICMP_NET_NOWAY, ICMP_HOST_NOWAY, ICMP_TOS_NET_UNREACH, ICMP_TOS_HOST_UNREACH,
		ICMP_FILTER_NOWAY, ICMP_HOST_PVERR, ICMP_PREC_CUTOFF, ICMP_MAX_DEST_UNREACH
	};

	// Redirect codes
	enum Redir {
		ICMP_NET_REDIR, ICMP_HOST_REDIR, ICMP_TOS_NET_REDIR, ICMP_TOS_HOST_REDIR,
		ICMP_MAX_REDIR
	};

	// Time exceed codes
	enum TtlExc {
		ICMP_IN_TRANSIT, ICMP_IN_REASS, ICMP_MAX_TTL_EXC
	};

	// Parameter problem codes
	enum ParamErr {
		ICMP_IP_HDR, ICMP_NEED_OPTION, ICMP_MAX_PARAM_ERR
	};

	uint8_t type;
	uint8_t code;
	uint16_t sum;
	// Message-specific data (ICMP payload) follows

	// Checksumming
	uint16_t Csum(uint len) {
		sum = 0;
		uint16_t csum = ipcksum((const uint16_t*)this, len, 0);
		csum = ~Htons(csum);
		if (!csum) --csum;
		return csum;
	}
	void SetCsum(uint len) { sum = Csum(len); }
	bool ValidateCsum(uint len) {
		const int16_t tmp = sum;
		const bool equal = (tmp == Csum(len));
		sum = tmp;
		return equal;
	}
		
	// Get ICMP payload (IP header, usually)
	uint8_t* GetEnclosed() { return (uint8_t*)this + sizeof (Icmph); }
};

// Magic cookies
#define ICMP_COOKIE_NETMASK  0x2a56

#endif // __ICMP_H__
