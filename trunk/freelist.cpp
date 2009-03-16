#include "enetkit.h"
#include "freelist.h"
#include "util.h"


// Note that this file doesn't use DMSG.
// That's because DMSG creates strings, which hits the string freelist,
// corrupting it.

FreelistVoidStar::FreelistVoidStar(uint16_t obsize, uint8_t n) :
	_head(NULL), _nummag(0), _size(obsize), _numkeep(n)
{
}


FreelistVoidStar::~FreelistVoidStar()
{
#ifdef DEBUG
	while (_head) xfree(exch(_head, _head->_next));
#endif
}


void* FreelistVoidStar::Alloc()
{
	Spinlock::Scoped L(_lock);

	Magazine* m = _head;

	while (m) {
		_malloc_region.Validate(m);
		if (m->_used != ~(native_uint_t)0) break;
		m = m->_next;
	}

	if (!m) m = Expand();

	const uint pos = Util::ffc(m->_used);
	assert(!(m->_used & (1 << pos)));

//	printf("Alloc: @%p[%d] (size %u)\n", m, pos, _size);

	m->_used |= 1 << pos;

	void* tmp = (void*)((uint8_t*)m + sizeof(Magazine) + ChunkSize() * pos + sizeof(Magazine*));
	_malloc_region.Validate(tmp);
	return tmp;
}


FreelistVoidStar::Magazine* FreelistVoidStar::Expand()
{
	_lock.AssertLocked();

	// Insert blank at front of chain so we find it first.
	Magazine* m = (Magazine*)xmalloc(sizeof(Magazine) + NATIVE_BITS * ChunkSize());
	_malloc_region.Validate(m);
	if (_head) _malloc_region.Validate(_head);
	m->_used = 0;
	m->_next = exch(_head, m);

	// Fill in back pointers to magazine, these precede each object
	uint8_t* ptr = (uint8_t*)m + sizeof(Magazine);
	for (uint i = 0; i < NATIVE_BITS; ++i) {
		*(Magazine**)ptr = m;
		ptr += sizeof(Magazine*) + _size;
	}

	++_nummag;
//	printf("Expand:  #%u @%p\n", _nummag, m);

	return m;
}


void FreelistVoidStar::Free(void* block)
{
	Spinlock::Scoped L(_lock);

	const uint8_t* mem = ((uint8_t*)block) - sizeof(Magazine*);
	Magazine* m = *(Magazine**)mem;

	_malloc_region.Validate(m);

	const uint pos = (mem - ((uint8_t*)(m+1))) / ChunkSize();

//	printf("Free:  @%p[%d] (size %u)\n", m, pos, _size);

	assert(m->_used & (1 << pos));
	m->_used &= ~(1 << pos);
	if (!m->_used) {
		if (m == _head) {
			if (_nummag <= _numkeep) return;
			_malloc_region.Validate(_head);
			_head = _head->_next;
			if (_head) _malloc_region.Validate(_head);
		} else {
			assert(_head);
			// Find the magazine prior to 'm' in chain
			_malloc_region.Validate(_head);
			Magazine* prev = _head;
			while (prev->_next && prev->_next != m) {
				_malloc_region.Validate(prev->_next);
				prev = prev->_next;
			}
			assert(prev);
			prev->_next = m->_next;
		}
//		printf("Release:  @%p, now down to %u\n", m, _nummag);
		xfree(m);
	}
}

void FreelistVoidStar::Check(void* ptr)
{
#ifdef DEBUG
		Mutex::Scoped L(_lock);
		for (Magazine* m = _head; m; m = m->_next)
			if ((uint8_t*)ptr >= (uint8_t*)m && (uint8_t*)ptr < (uint8_t*)m + _size)
				panic("attempt to free() member of freelist");
#endif
}
