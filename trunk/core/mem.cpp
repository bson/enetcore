#include "enetkit.h"


static MemStats memstats = { 0, 0, 0 };

static __noreturn void OutOfMemory()
{
	_malloc_region.SetReserve(0);
	panic("Out of memory");
}


void *xmalloc(uint size)
{
	++memstats.num_malloc;
	void* tmp = malloc(size);
	if (!tmp) OutOfMemory();
	assert(!IsLiteral(tmp));
	_malloc_region.Validate(tmp);
	return tmp;
}


void *xrealloc(void *old, uint size)
{
	if (IsLiteral(old)) return xmalloc(size);

	++memstats.num_realloc;
	if (old) _malloc_region.Validate(old);
	void* tmp = realloc(old, size);
	if (!tmp) OutOfMemory();
	assert(!IsLiteral(tmp));
	_malloc_region.Validate(tmp);
	return tmp;
}


void *xmemdup(const void* block, uint size)
{
	if (IsLiteral(block)) return const_cast<void*>(block);

	void *tmp = xmalloc(size);
	assert(!IsLiteral(tmp));
	memcpy(tmp, block, size);
	_malloc_region.Validate(tmp);
	return tmp;
}


uchar* xstrdup(const uchar* s)
{
	if (IsLiteral(s))  return const_cast<uchar*>(s);

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
	if (_malloc_region.IsInRegion(ptr)) {
		_malloc_region.Validate(ptr);
		++memstats.num_free;
		free(ptr);
	}
}

const MemStats& xmemstats() { return memstats; }
