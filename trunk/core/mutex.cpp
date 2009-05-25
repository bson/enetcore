#include "enetkit.h"
#include "mutex.h"
#include "thread.h"


bool Mutex::TryLock() const 
{
	Spinlock::Scoped L(_lock);
	if (_count) return false;
		
	_tid = &Self();
	++_count;
	return true;
}


void Mutex::Lock() const
{
	Spinlock::Scoped L(_lock);
	while (_count && _tid != &Self()) {
		++_waiters;
		_lock.Unlock();
		Self().WaitFor(this);
		_lock.Lock();
		assert(_waiters);
		--_waiters;
	}

	assert((!_tid && !_count) || (_count && _tid == &Self()));
	_tid = &Self();
	++_count;
}


void Mutex::Unlock() const
{
	AssertLocked();

	if (!--_count) _tid = 0;
	if (_waiters) Self().WakeSingle(this);
}


void CondVar::Wait(Mutex& m)
{
	m.AssertLocked();

	++_count;

	// Temporarily release mutex regardless of recursion depth
	const uint count = exch<uint16_t>(m._count, 1);
	m.Unlock();

	Self().WaitFor(this);

	m.Lock();
	assert(_count);
	--_count;

	m._count = count;
}


void CondVar::Wait(Mutex& m, const Time& delay)
{
	m.AssertLocked();

	++_count;

	// Temporarily release mutex regardless of recursion depth
	const uint count = exch<uint16_t>(m._count, 1);
	m.Unlock();

	Self().WaitFor(this, Time::Now() + delay);

	m.Lock();
	assert(_count);
	--_count;
	m._count = count;
}


void EventObject::Set(uint8_t new_state)
{
	_lock.Lock();
	const uint8_t prev_state = exch(_state, new_state);
	_lock.Unlock();
	if (new_state && new_state != prev_state) {
		if (_evob) _evob->Set();

		if (_count) {
			if (_mode == SELF_RESET)
				Self().WakeSingle(this);
			else
				Self().WakeAll(this);
		}
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
