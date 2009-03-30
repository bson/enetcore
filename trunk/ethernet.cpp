#include "enetkit.h"
#include "ethernet.h"


EventObject _net_event;

Ethernet _eth0(CS8900A_BASE);


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


} // namespace BufferPool



Ethernet::Ethernet(uint32_t base)
{
	_base = (volatile uint32_t*)base;
}


void Ethernet::Initialize()
{
}


IOBuffer* Ethernet::Recv()
{
	Spinlock::Scoped L(_lock);
	if (_recvq.Empty()) return NULL;

	IOBuffer* buf = _recvq.Front();
	_recvq.PopFront();
	return buf;
}


void Ethernet::Send(IOBuffer* buf)
{
	Spinlock::Scoped L(_lock);

	_sendq.PushBack(buf);
	FillTx();
}


void Ethernet::FillTx()
{
}


// * static __irq NAKED
void Ethernet::Interrupt()
{
	SaveStateExc(4);

	_eth0.HandleInterrupt();

	_vic.ClearPending();
	LoadStateReturnExc();
}


void Ethernet::HandleInterrupt()
{
}
