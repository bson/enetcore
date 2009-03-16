#ifndef __UTIL_H__
#define __UTIL_H__

#include "arc4.h"


// These are higher-order utility functions

namespace Util {
	int ToUpper(int c);
	int ParseHexDigit(int c);

	const uint AppendVFmt(Vector<uchar>& dest, const uchar* fmt, va_list& va);
	inline uint AppendFmt(Vector<uchar>& dest, const uchar* fmt, ...) {
		va_list va;
		va_start(va, fmt);
		const uint tmp = AppendVFmt(dest, fmt, va);
		va_end(va);
		return tmp;
	}

	// Flags for FormatNumber
	enum { FMT_UNSIGNED = 1 };
	void FormatNumber(Vector<uchar>& dest, uint64_t val, uint flags, uint radix, uint digits = 0);

	void Trim(Deque<uchar>& buffer);

	// Find first clear bit in a word
	template <typename T> uint ffcw(T w) {
		T mask = ~(T)0;
		uint first = 0;
		uint width = sizeof(T) * 8;

		while (width) {
			if (!(w & (mask << first)))  return first;
			width /= 2;
			mask >>= width;

			if ((w & (mask << first)) == (mask << first)) first += width;
		}
		return NOT_FOUND;
	}

	// Find first set
	template <typename T> uint ffsw(T w) {
		T mask = ~(T)0;
		uint first = 0;
		uint width = sizeof(T) * 8;

		while (width) {
			if ((w & (mask << first)) == (mask << first)) return first;
			width /= 2;
			mask >>= width;

			if (!(w & (mask << first))) first += width;
		}
		return NOT_FOUND;
	}

	inline uint ffc(native_uint_t w) { return ffcw<native_uint_t>(w); }
	inline uint ffs(native_uint_t w) { return ffsw<native_uint_t>(w); }

	// Returns unique computer ID - digest of a Mac address
	struct ComputerID {
		uint8_t id[20];

		ComputerID();
	};

	const ComputerID& GetComputerID();

	// Straightforward pointer ordering
	bool VoidStarOrder(const void* a, const void* b);

	// Bring random number feature into Util namespace
	inline void RandomSeed(const uint8_t* key, uint key_len) { Arc4::RandomSeed(key, key_len); }
	template <typename T> inline T Random() { return Arc4::Random<T>(); }
}

#endif	// __UTIL_H__
