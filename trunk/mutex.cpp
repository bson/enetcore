#include "enetkit.h"
#include "mutex.h"
#include "thread.h"


void CondVar::Wait(Mutex& m)
{
	m.AssertLocked();

	// Temporarily release mutex regardless of recursion depth
	m._tid = &Thread::Self();
	const uint count = m._count;
	m._count = 1;
	m.Unlock();

	Thread::Self().WaitFor(this);

	m.Lock();
	m._tid = &Thread::Self();
	m._count = count;
}


void CondVar::Wait(Mutex& m, const Time& delay)
{
	m.AssertLocked();

	// Temporarily release mutex regardless of recursion depth
	m._tid = &Thread::Self();
	const uint count = m._count;
	m._count = 1;
	m.Unlock();

	Thread::Self().WaitFor(this, Time::Now() + delay);

	m.Lock();
	m._tid = &Thread::Self();
	m._count = count;
}
