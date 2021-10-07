// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __PLATFORM_H__
#define __PLATFORM_H__


#define abort() (panic("ABORT"), (void)0)

namespace Platform {

	// Memory regions
	class Region {
		uint8_t* _start;		// Start of region
		uint8_t* _end;			// End of region
		uint8_t* _next;			// Next available location (sbrk style)
		uint _reserve;			// Amount to hold in reserve (for panic etc)
	public:
		// For enetcore we have a static initializer for the constant case
		Region(uint size, uint8_t* start) :
			_start(start), _end(start + size), _next(start), _reserve(0)
		{ }

		// Ctor does nothing; this is to allow use of malloc() from
		// static initializers without creating an initializer
		// ordering dependency.  The GetCore() function will
		// autoinitialize.  For this reason, only GetCore() should
		// really be used from ctor's, either directly (e.g. to
		// allocate a thread stack from a stack region), or indirectly
		// (e.g. malloc, from the malloc region).
			
		Region();
		void Init(uint size, uint8_t* start = NULL);
		~Region() { }

		// Get a block of memory
		void* GetMem(int amount);

        // Get a block of memory with a given alignment
        void* GetAlignedMem(int amount, uint alignment);

		// Wrapper that also auto-inits; useful when called from static ctor's
		inline void* GetCore(int amount, uint size, uint8_t* start = NULL) {
            ScopedNoInt G;

			if (!_start)
                Init(size, start);

			if (!_start)
                abort();

			return GetMem(amount);
		}
			
		// Page size.  Malloc wants this for its aligned alloc ops. We don't
		// particularly care, so just tell it something reasonable.
		uint GetPageSize() { return 4096; }

		// Get free space
		uint GetFreeMem() {
            ScopedNoInt G;
            return _end - _next - _reserve;
        }

		// Get total space
		uint GetSize() {
            ScopedNoInt G;
            return _end - _start;
        }

		// Set reserve
		// The reserve allows us to set aside memory to gracefully terminate
		void SetReserve(uint reserve);

		// Get start of, end of (first byte following) region
		[[__finline]] inline void* GetStart() const { return _start; }
		[[__finline]] inline void* GetEnd() const { return _end; }

		// True if pointer falls in region
		bool IsInRegion(const void* ptr) {
            ScopedNoInt G;
			return (uint8_t*)ptr >= _start && (uint8_t*)ptr < _end;
		}

		// Validate pointer as being valid for used part of region
		void Validate(void* ptr) const {
#ifdef DEBUG
            ScopedNoInt G;

			// Regions are needed by assert(), so can't use it here
			if (!_start)
                abort();

			if (_next > _end)
                abort();

			if ((uint8_t*)ptr < _start)
                abort();

			if ((uint8_t*)ptr >= _next) 
                abort();
// Redundant
//		if ((uint8_t*)ptr >= _end) abort();
#endif
		}

		// Print debug message about arena
		void DebugMsg(const class String& name) {
#ifdef DEBUG
			DMSG("%S region: %p-%p, %uk (%u bytes held in reserve)",
				 &name, _start, _end, (uint)(_end-_start)/1024, _reserve);
#endif
		}

		// Round up address; size is power of two
		inline uintptr_t RoundUpAddr(uintptr_t addr, uint size) {
			assert(!(size & (size - 1))); // verify that size is power of two
			return (addr + size - 1) & ~(size - 1);
		}

		inline uintptr_t RoundUpPage(uintptr_t addr) { return RoundUpAddr(addr, GetPageSize()); }
	};

	extern Region _malloc_region, _data_region, _iram_region,
		_text_region, _xflash_region;

	// Print debug output regarding malloc stats
	void DebugMallocMsg();

	// Get free malloc memory
	uint GetFreeMem();

	// Get a MAC address
    // XXX should come out of EEPROM
	inline bool GetMacAddr(uint8_t macaddr[6]) {
		memcpy(macaddr, "\0eaddd", 6);
		return true;
	}
}

using Platform::_malloc_region;
using Platform::_iram_region;
using Platform::_data_region;
using Platform::_text_region;
using Platform::_xflash_region;


// Various compatibility functions
int atoi(const char* s);		// in platform.cpp

[[__finline]] inline int xatoi(const uchar* s) {
    return ::atoi((const char*)s);
}

in_addr_t inet_addr(const char* a);

[[__finline]] inline in_addr_t xinet_addr(const uchar* a) {
     return ::inet_addr((const char*)a);
}

long strtol(const char* __restrict str, char **__restrict endptr, int base);
int64_t strtoll(const char* __restrict str, char **__restrict endptr, int base);

#if 0
inline uchar* xinet_ntop(int af, const void* __restrict src, uchar* __restrict dst, uint len) {
	return (uchar*)::inet_ntop(af, src, (char*)dst, len);
}
#endif

#ifdef USE_LITERALS
// Test if location is literal in text
inline bool IsLiteral(const void* p) { return p && _text_region.IsInRegion(p); }
#else
inline bool IsLiteral(const void*)  { return false; }
#endif


// If pointer refers to malloc heap, validate
inline void VALIDATE_INUSE(void* ptr)  {
#ifdef MEMDEBUG
	if (_malloc_region.IsInRegion(ptr)) {
		_malloc_region.Validate(ptr);
		malloc_check_inuse(findmblk(ptr));
	}
#endif
}

inline void VALIDATE_FREE(void* ptr)  {
#ifdef MEMDEBUG
#if 0
	if (_malloc_region.IsInRegion(ptr))  malloc_check_free((uint8_t*)ptr - 8);
#endif
#endif
}

#endif // __PLATFORM_H__
