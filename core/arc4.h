// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __ARC4_H__
#define __ARC4_H__


class Arc4 {
	uint8_t _S[256];
	uint8_t _x, _y;

	// For generating a random number
	static Arc4 _random;

public:
	Arc4(const uint8_t* key = NULL, uint key_len = 0);

	void Init(const uint8_t* key, uint key_len);

	// Process block
	void Process(const uint8_t* __restrict from, uint8_t* __restrict to, uint len);

	// Initialize static object used for random number generation
	static void RandomSeed(const uint8_t* key, uint key_len) { _random.Init(key, key_len); }

	// Generate a random integer of type T
	template <typename T> static T Random() {
		T v = 0;
		T r;
		_random.Process((const uint8_t*)&v, (uint8_t*)&r, sizeof r);
		return r;
	}
};

#endif // __ARC4_H__
