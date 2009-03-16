#ifndef __PLATFORM_H__
#define __PLATFORM_H__

extern "C" {
void abort();
}

namespace Platform {
	class Spinlock {
	public:
		Spinlock() { }
		~Spinlock() { }
		uint32_t Lock() const { return DisableInterrupts(); }
		void Unlock(uint32_t cpsr) const { EnableInterrupts(cpsr); }
		void AssertLocked() const { }

		class Scoped {
			const Spinlock& _lock;
			uint _cpsr;
		public:
			Scoped(const Spinlock& lock) : _lock(lock) { _cpsr = _lock.Lock(); }
			~Scoped() { _lock.Unlock(_cpsr); }
		};
	};

	// Memory regions
	class Region {
		uint8_t* _start;		// Start of region
		uint8_t* _end;			// End of region
		uint8_t* _next;			// Next available location (sbrk style)
		uint _reserve;			// Amount to hold in reserve (for panic etc)
		Spinlock _lock;
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

		// Wrapper that also auto-inits; useful when called from static ctor's
		inline void* GetCore(int amount, uint size, uint8_t* start = NULL) {
			Spinlock::Scoped L(_lock);
			if (!_start)  Init(size, start);
			if (!_start) abort();
			return GetMem(amount);
		}
			
		// Page size.  Malloc wants this for its aligned alloc ops. We don't
		// particularly care, so just tell it something reasonable.
		uint GetPageSize() { return 4096; }

		// Get free space
		uint GetFreeMem() { Spinlock::Scoped L(_lock); return _end - _next - _reserve; }

		// Get total space
		uint GetSize() { Spinlock::Scoped L(_lock); return _end - _start; }

		// Set reserve
		// The reserve allows us to set aside memory to gracefully terminate
		void SetReserve(uint reserve);

		// True if pointer falls in region
		bool IsInRegion(void* ptr) {
			Spinlock::Scoped L(_lock);
			return (uint8_t*)ptr >= _start && (uint8_t*)ptr < _end;
		}

		// Validate pointer as being valid for used part of region
		void Validate(void* ptr) const {
#ifdef DEBUG
			Spinlock::Scoped L(_lock);
			// Regions are needed by assert(), so can't use it here
			if (!_start) abort();
			if (_next > _end) abort();
			if ((uint8_t*)ptr < _start) abort();
			if ((uint8_t*)ptr >= _next) abort();
// Redundant
//		if ((uint8_t*)ptr >= _end) abort();
#endif
		}

#if 0
		// Print debug message about arena
		void DebugMsg(const String& name) {
#ifdef DEBUG
			DMSG("%S region: %p-%p, %uk (%u bytes held in reserve)",
				 &name, _start, _end, (uint)(_end-_start)/1024, _reserve);
#endif
		}
#endif
		// Round up address; size is power of two
		inline uintptr_t RoundUpAddr(uintptr_t addr, uint size) {
			assert(!(size & (size - 1))); // verify that size is power of two
			return (addr + size - 1) & ~(size - 1);
		}

		inline uintptr_t RoundUpPage(uintptr_t addr) { return RoundUpAddr(addr, GetPageSize()); }
	};


	extern Region _malloc_region, _data_region, _stack_region,
		_text_region, _xflash_region;

#if 0
	// Print debug output regarding malloc stats
	void DebugMallocMsg();
#endif

	// Get free malloc memory
	uint GetFreeMem();

	// Get a MAC address
	inline bool GetMacAddr(uint8_t macaddr[6]) {
		memcpy(macaddr, "deaddd", 6);
		return true;
	}
}

using Platform::Spinlock;

using Platform::_malloc_region;
using Platform::_stack_region;
using Platform::_data_region;
using Platform::_text_region;
using Platform::_xflash_region;


// If pointer refers to malloc heap, validate
inline void VALIDATE_INUSE(void* ptr)  {
#ifdef DEBUG
	if (_malloc_region.IsInRegion(ptr)) {
		_malloc_region.Validate(ptr);
		malloc_check_inuse(ptr);
	}
#endif
}

inline void VALIDATE_FREE(void* ptr)  {
#ifdef DEBUG
	if (_malloc_region.IsInRegion(ptr))  malloc_check_free(ptr);
#endif
}

#endif // __PLATFORM_H__
