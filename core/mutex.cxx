// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetkit.h"
#include "core/mutex.h"
#include "core/thread.h"


bool Mutex::TryLock() const 
{
	AssertNotInterrupt();

    ScopedNoInt G;

	if (_count)
        return false;
		
	_tid = Thread::GetCurThread();
	++_count;
	return true;
}


void Mutex::Lock() const
{
	AssertNotInterrupt();

    ScopedNoInt G;

	while (_count && _tid != Thread::GetCurThread()) {
		++_waiters;
		Thread::WaitFor(this);
		assert(_waiters);
		--_waiters;
	}

	assert((!_tid && !_count) || (_count && _tid == Thread::GetCurThread()));
	_tid = Thread::GetCurThread();
	++_count;
}


void Mutex::Unlock() const
{
	AssertNotInterrupt();

	AssertLocked();

	if (!--_count)
        _tid = 0;

	if (_waiters)
        Thread::WakeSingle(this);
}


void CondVar::Wait(Mutex& m)
{
	m.AssertLocked();

	++_count;

	// Temporarily release mutex regardless of recursion depth
	const uint count = exch<uint16_t>(m._count, 1);
	m.Unlock();

	Thread::WaitFor(this);

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

	Thread::WaitFor(this, Time::Now() + delay);

	m.Lock();
	assert(_count);
	--_count;
	m._count = count;
}


void EventObject::Set(uint8_t new_state)
{
    uint8_t prev_state;

    {
        ScopedNoInt G;
        prev_state = exch(_state, new_state);
    }

    if (new_state && new_state != prev_state) {
		if (_count) {
			if (_mode == SELF_RESET)
				Thread::WakeSingle(this);
			else
				Thread::WakeAll(this);
		}
	}
}


void EventObject::Wait()
{
    ScopedNoInt G;

	while (!_state) {
		++_count;
		Thread::WaitFor(this);
		assert(_count);
		--_count;
	}
	if (_mode == SELF_RESET)
        _state = 0;
}


bool EventObject::Wait(Time delay)
{
	const Time deadline = Time::Now() + delay;

    ScopedNoInt G;

	while (!_state && Time::Now() < deadline) {
		++_count;

		Thread::WaitFor(this, deadline);

		assert(_count);

		--_count;
	}
	const bool retval = _state != 0;

	if (_mode == SELF_RESET)
        _state = 0;

	return retval;
}


uint8_t EventObject::GetState() const
{
    ScopedNoInt G;
	return _state;
}
