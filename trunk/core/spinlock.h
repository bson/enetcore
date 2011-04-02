#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

class Spinlock {
	mutable uint32_t _cpsr;
	mutable uint _count;

public:
	Spinlock() : _count(0) { }
	~Spinlock() { }
	void Lock() { const uint32_t cpsr = DisableInterrupts(); if (!_count++) _cpsr = cpsr; }
	void Unlock() {
		assert((int)_count > 0);
		if (!--_count) EnableInterrupts(_cpsr);
	}
	void AssertLocked() const { assert(_count); }

	// Abandon lock - must be held.  This resets the lock, which must be held once
	// by the caller.
	void Abandon() { assert(_count == 1);  _count = 0; }

	class Scoped {
		Spinlock& _lock;
	public:
		Scoped(const Spinlock& lock) : _lock((Spinlock&)lock) { _lock.Lock(); }
		~Scoped() { _lock.Unlock(); }
	};
};


#endif // __SPINLOCK_H__
