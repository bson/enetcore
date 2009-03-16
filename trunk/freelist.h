#ifndef __FREELIST_H__
#define __FREELIST_H__

// #include "mutex.h"


// Free list
// Use to reduce heap thrashing for short-lived objects.
// Don't use for long-lived objects or it will become horribly fragmented;
// instead use xmalloc() for those.

class FreelistVoidStar {
	// Magazine header - this is followed by 32/64 objects
	struct Magazine {
		native_uint_t _used;	// Mask of entries used
		Magazine* _next;		// Linked list
	};
		
	Spinlock _lock;
	Magazine* _head;			// First magazine in list
	uint _nummag;				// Number of magazines on list
	uint16_t _size;				// Object size
	uint8_t _numkeep;			// Number of magazines to keep

public:
	// n is number of magazines to retain permanently
	FreelistVoidStar(uint16_t obsize, uint8_t n = 1);
	~FreelistVoidStar();

	// Allocate an object
	void* Alloc();

	// Free an object
	void Free(void* block);

	void Check(void* ptr);
private:
	// Add a mag to chain; returns new mag
	Magazine* Expand();

	// Get object allocation size
	uint ChunkSize() const { return sizeof (Magazine*) + _size; }
};


// Type wrapper - handles type conversions without templatizing the entire
// implementation.
template <typename T> class Freelist: public FreelistVoidStar {
public:
	Freelist(uint numkeep = 1) : FreelistVoidStar(sizeof(T), numkeep) { assert(numkeep); }
	~Freelist() { }

	T* Alloc() { return ::new(FreelistVoidStar::Alloc()) T(); }
	T* Alloc(const T& arg) { return ::new(FreelistVoidStar::Alloc()) T(arg); }
	void Free(T* ptr) { ptr->~T(); FreelistVoidStar::Free(ptr); }

	// For more specialized constructors - returns uninitialized memory
	T* AllocMem() { return (T*)FreelistVoidStar::Alloc(); }

	// Check pointer to see if we're attempting to free something on this list
	void Check(void* ptr) { FreelistVoidStar::Check(ptr); }
};

#endif // __FREELIST_H__
