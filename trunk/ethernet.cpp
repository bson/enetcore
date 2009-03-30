#include "enetkit.h"
#include "ethernet.h"


namespace BufferPool {

static Spinlock _lock;

static Vector<IOBuffer*> _pool;

void Initialize(uint num)
{
	Spinlock::Scoped L(_lock);

	_pool.Reserve(num);
	for (uint i = 0; i < num; ++i)
		_pool.PushBack(new IOBuffer(1600));
}


IOBuffer* Alloc()
{
	Spinlock::Scoped L(_lock);

	if (_pool.Empty()) return NULL;

	IOBuffer* buf = _pool.Back();
	_pool.PopBack();
	return buf;
}


void FreeBuffer(IOBuffer* buf)
{
	Spinlock::Scoped L(_lock);

	_pool.PushBack(buf);
}


}
