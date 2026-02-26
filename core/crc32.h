// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

// Derived from CRC32 algorithm in RFC1952
// Public domain

// Don't include this directly - use board.h to use HW CRC when available

#ifndef __CRC32_H__
#define __CRC32_H__

#ifndef HAVE_HW_CRC

// Define to dynamically initialize table.  Mainly useful to generate the table
// in the first place.  A precomputed table is const, so will be placed in the
// text segment.

// #define DYNAMIC_CRC32
// #define OUTPUT_TABLE			// Also print table on stdout

class Crc32 {
	// Table of CRCs of all 8-bit messages.
#ifndef DYNAMIC_CRC32
	const
#endif
	static uint32_t _crc_table[256];

	uint32_t _crc;
public:
	Crc32() : _crc(~0) { }

#ifdef DYNAMIC_CRC32
	static void InitTable();
#endif
	void Update(const void* __restrict block, uint len) __restrict;
	uint32_t GetValue() const { return ~_crc; }

	// Convenience function to checksum a block
	static uint32_t Checksum(const void* block, uint len);
};

#endif  // HAVE_HW_CRC

#endif // __CRC32_H__
