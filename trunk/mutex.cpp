#include "enetkit.h"
#include "mutex.h"
#include "thread.h"


void CondVar::Wait(Mutex& m)
{
	m.AssertLocked();

	// Temporarily release mutex regardless of recursion depth
	const uint count = exch<uint>(m._count, 1);
	m.Unlock();

	Self().WaitFor(this);

	m.Lock();
	m._count = count;
}


void CondVar::Wait(Mutex& m, const Time& delay)
{
	m.AssertLocked();

	// Temporarily release mutex regardless of recursion depth
	const uint count = exch<uint>(m._count, 1);
	m.Unlock();

	Self().WaitFor(this, Time::Now() + delay);

	m.Lock();
	m._count = count;
}


void EventObject::Set(uint8_t new_state)
{
	_lock.Lock();
	const uint8_t prev_state = exch(_state, new_state);
	_lock.Unlock();
	if (new_state && new_state != prev_state && _count) {
		if (_mode == SELF_RESET)
			Self().WakeSingle(this);
		else
			Self().WakeAll(this);
	}
}


void EventObject::Wait()
{
	Spinlock::Scoped L(_lock);

	while (!_state) {
		++_count;
		_lock.Unlock();
		Self().WaitFor(this);
		_lock.Lock();
		assert(_count);
		--_count;
	}
	if (_mode == SELF_RESET) _state = 0;
}


bool EventObject::Wait(Time delay)
{
	const Time deadline = Time::Now() + delay;

	Spinlock::Scoped L(_lock);

	while (!_state && Time::Now() < deadline) {
		++_count;
		_lock.Unlock();
		Self().WaitFor(this, deadline);
		_lock.Lock();
		assert(_count);
		--_count;
	}
	const bool retval = _state != 0;
	if (_mode == SELF_RESET) _state = 0;
	return retval;
}


uint8_t EventObject::GetState() const
{
	Spinlock::Scoped L(_lock);
	return _mode;
}
