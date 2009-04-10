#ifndef __IPCONN_H__
#define __IPCONN_H__

#include "lookup3.h"


// Endpoint tuple

struct Tuple {

	in_addr_t saddr;
	in_addr_t daddr;
	uint16_t sport;
	uint16_t dport;

	bool operator==(const Tuple& arg) const { return !memcmp(this, &arg, sizeof (Tuple)); }

	// Hash tuple
	static void Hash(const void* vt, uint32_t& hash1, uint32_t& hash2) {
		assert(vt);
		const Tuple& t = *(Tuple*)vt;
		Lookup3::hashlittle2(&t, sizeof t, &hash1, &hash2);
	}

	static bool HashEqual(const void* a, const void* b) {
		assert(a);
		assert(b);
		return *(Tuple*)a == *(Tuple*)b;
	}
};

#endif // __IPCONN_H__
