// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"

#undef malloc
#undef realloc
#undef free

static MemStats memstats = { 0, 0, 0 };

[[noreturn]] static void OutOfMemory()
{
	_malloc_region.SetReserve(0);
	panic("Out of memory");
}


#ifdef MEMDEBUG
enum { MALLOC_MAGIC = 0xa0d00dad };

// Prefix put in allocated memory block
struct MemDebug {
	uint32_t magic;
	uint32_t size;				// Allocation size (including this debug area)

	MemDebug(uint size) : magic(MALLOC_MAGIC), size(size) { }
	void Validate(const void* ptr) const {
		if (magic != MALLOC_MAGIC)
            abort();

		if ((const uint8_t*)ptr > (const uint8_t*)this + size)
            abort();
	}
};
#endif


void *xmalloc(uint size)
{
	AssertNotInterrupt();

	++memstats.num_malloc;
#ifdef MEMDEBUG
	size += sizeof (MemDebug);
#endif
	void* tmp = malloc(size);
	if (!tmp)
        OutOfMemory();

	assert(!IsLiteral(tmp));
	_malloc_region.Validate(tmp);
#ifdef MEMDEBUG
	new (tmp) MemDebug(size);
	tmp = (uint8_t*)tmp + sizeof (MemDebug);
#endif
	return tmp;
}


void *xrealloc(void *old, uint size)
{
	AssertNotInterrupt();

	if (!old)
        return xmalloc(size);

    // Can't realloc a literal - we don't know how big it is
    assert(!IsLiteral(old));

#ifdef MEMDEBUG
	old = (uint8_t*)old - sizeof (MemDebug);
	size += sizeof (MemDebug);
#endif
	++memstats.num_realloc;
	_malloc_region.Validate(old);

	void* tmp = realloc(old, size);
	if (!tmp)
        OutOfMemory();

	assert(!IsLiteral(tmp));

	_malloc_region.Validate(tmp);
#ifdef MEMDEBUG
	new (tmp) MemDebug(size);
	tmp = (uint8_t*)tmp + sizeof (MemDebug);
#endif
	return tmp;
}


void *xmemdup(const void* block, uint size)
{
	void *tmp = xmalloc(size);
	assert(!IsLiteral(tmp));
	memcpy(tmp, block, size);
	_malloc_region.Validate(tmp);
	return tmp;
}


uchar* xstrdup(const uchar* s)
{
	if (IsLiteral(s))
        return const_cast<uchar*>(s);

	const int len = xstrlen(s);
	uchar* tmp = (uchar*)xmemdup(s, len+1);
	_malloc_region.Validate(tmp);
	return tmp;
}


uchar* xstrndup(const uchar* s, uint num)
{
	const int len = min<int>(num, xstrlen(s));
	uchar* tmp = (uchar*)xmalloc(len + 1);
	assert(!IsLiteral(tmp));
	if (len)
		memcpy(tmp, s, len);
	tmp[len] = 0;
	_malloc_region.Validate(tmp);
	return tmp;
}


uchar* xmemtostr(const void* block, uint size)
{
	uchar *tmp = (uchar*)xmalloc(size + 1);
	assert(!IsLiteral(tmp));
	if (size)
		memcpy(tmp, block, size);
	tmp[size] = 0;
	_malloc_region.Validate(tmp);
	return tmp;
}


void xxfree(void* ptr)
{
	AssertNotInterrupt();
	if (_malloc_region.IsInRegion(ptr)) {
		VALIDATE_INUSE(ptr);
#ifdef MEMDEBUG
		ptr = (uint8_t*)ptr - sizeof (MemDebug);
		((MemDebug*)ptr)->Validate(ptr);
#endif
		++memstats.num_free;
		free(ptr);
	}
}


#ifdef MEMDEBUG
void* findmblk(void* ptr)
{
	uint32_t* u = (uint32_t*)ptr;
	while (_malloc_region.IsInRegion(u) && *u != MALLOC_MAGIC)
		--u;

	const MemDebug& d = *(MemDebug*)u;
	d.Validate(ptr);
	return u;
}
#endif

const MemStats& xmemstats() { return memstats; }

// Explicitly emit these for gcc, it needs them sometimes for struct
// copies and initialization.
#undef memset
void* memset(void* b, int c, size_t n) {
    return xmemset(b, c, n);
}
    
extern inline void* memcpy(void*, const void*, size_t);
