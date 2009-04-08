#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "thread.h"

template <typename T> class ScopedLock {
	const T& _lock;
public:
	ScopedLock(const T& l) : _lock(l) { _lock.Lock(); }
	~ScopedLock() { _lock.Unlock(); }
private:
	ScopedLock();
	ScopedLock(const ScopedLock&);
	ScopedLock& operator=(const ScopedLock&);
};


class Mutex {
protected:
	friend class CondVar;
	friend class SecondLock;

	mutable Spinlock _lock;
	mutable ThreadId _tid;
	mutable uint _count;

public:
	Mutex() {
		_tid = 0;
		_count = 0;
	}

	virtual ~Mutex() {  }

	void AssertLocked() const {
		assert(_count);
		assert(_tid == &Self());
	}

	bool TryLock() const {
		Spinlock::Scoped L(_lock);
		if (_count) return false;
		
		_tid = &Self();
		++_count;
		return true;
	}

	void Lock() const {
		Spinlock::Scoped L(_lock);
		while (_count && _tid != &Self()) {
			_lock.Unlock();
			Self().WaitFor(this);
			_lock.Lock();
		}

		assert((!_tid && !_count) || (_count && _tid == &Self()));
		_tid = &Self();
		++_count;
	}

	void Unlock() const { 
		AssertLocked();

		if (!--_count) _tid = 0;
		Self().WakeSingle(this);
	}

	typedef ScopedLock<Mutex> Scoped;

private:
	// These make no sense
	Mutex(const Mutex&);
	Mutex& operator=(const Mutex&);
};


// We have lock 1.  Acquire lock 2.
// While RAII (like a Scoped<T>), it's also a lock - it supports Lock/Unlock primitives.
// This makes it useable, for instance, as an object lock.

class SecondLock {
	mutable Mutex& _lock1;
	mutable Mutex& _lock2;
public:
	SecondLock(const volatile Mutex& m1, const volatile Mutex& m2) :
		_lock1(const_cast<Mutex&>(m1)),
		_lock2(const_cast<Mutex&>(m2)) {
		Lock();
	}
	~SecondLock() { Unlock(); }

	void Lock() volatile const;
	void Unlock() volatile const;
	void AssertLocked() volatile const;

private:
	SecondLock();
	SecondLock(const SecondLock&);
	SecondLock& operator=(const SecondLock&);
};


class CondVar {
public:
	CondVar() {  }
	virtual ~CondVar() {  }

	void Wait(Mutex& m, const Time& delay);
	void Wait(Mutex& m);
	void Signal() { Self().WakeSingle(this); }
	void Broadcast() { Self().WakeAll(this); }
private:
	// These make no sense
	CondVar(const CondVar&);
	CondVar& operator=(const CondVar&);
};


// Like the Win32 EventObject
class EventObject {
public:
	enum Mode { SELF_RESET, MANUAL_RESET };
private:
	mutable Spinlock _lock;
	uint8_t _state;
	uint8_t _mode;
public:
	EventObject(uint8_t state = 0, Mode mode = SELF_RESET) :
		_state(state), _mode(mode)
	{
	}

	~EventObject() { }

	void Set(uint8_t new_state = 1);
	void Reset() { Set(0); }
	void Wait();
	bool Wait(Time delay);
	uint8_t GetState() const;
};

#endif	// __MUTEX_H__

