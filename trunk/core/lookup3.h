#ifndef __LOOKUP3_H__
#define __LOOKUP3_H__

namespace Lookup3 {
	
	// Hash an array of uint32_t keys
	// Returns single hash
	uint32_t hashword(const uint32_t *keys, uint len, uint32_t initval = 0);

	// Like hashword, but returns two hashes
	void hashword2(const uint32_t *keys, uint len, uint32_t* pc, uint32_t* pb);

	// Hash memory block
	// Returns single hash
	uint32_t hashlittle(const void *key, uint length, uint32_t initval = 0);

	// Hash memory block
	// Returns two hashes (pc is better)
	void hashlittle2(const void *key, uint length, uint32_t *pc, uint32_t *pb);
		
	// Self test
#ifdef DEBUG
	void selftest();
#endif
}
#endif // __LOOKUP3_H__
