// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "arc4.h"

Arc4 Arc4::_random;


Arc4::Arc4(const uint8_t* key, uint key_len)
{
	if (key && key_len)
		Init(key, key_len);
}


void Arc4::Init(const uint8_t* key, uint key_len)
{
	for (uint i = 0; i < 256; ++i)
		_S[i] = i;

	_x = 1;
	_y = 0;

	// Initialize vector
	uint8_t j = 0;
	int k = 0;

	for(uint i = 0; i < 256; ++i) {
		const uint8_t a = _S[i];
		j += key[k] + a;
		_S[i] = exch(_S[j], a);
		if ((uint)++k >= key_len)  k = 0;
	}
}


void Arc4::Process(const uint8_t* __restrict from, uint8_t* __restrict to, uint len)
{
	while (len--) {
		const uint8_t a = _S[_x];
		_y += a;
		const uint8_t b = exch(_S[_y], a);
		_S[_x++] = b;
		*to++ = *from++ ^ _S[(uint8_t)(a + b)];
	}
}
