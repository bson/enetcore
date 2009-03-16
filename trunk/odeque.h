#ifndef __ODEQUE_H__
#define __ODEQUE_H__


// Ordered deque - basically a priority queue

// The only way to add items is using Insert()
// Like STL, requires T to implement bool operator>(const T&) const

// Greater items insert towards the back, lesser items towards the front.
// Equal items sort by insertion order.

// Safe for POD only

template <typename T, OrderFunc Order = T::Order> class ODeque {
	OVector<T,Order> _v;
	uint _head;					// Start of "ring"

	typedef ODeque<T,Order> Self;

public:
	ODeque() { _head = 0; }
	ODeque(uint reserve) : _v(reserve) { _head = 0; }
	ODeque(const Self& arg) : _v(arg._v) { _head = arg._head; }

	virtual ~ODeque() { };

	uint Headroom() const { return _v.Headroom() + _head; }

	void Compact() {
		if (_head) {
			if (_head < _v.Size())
				move(&_v[0], _v + _head, _v.Size() - _head);

			_v.SetSize(_v.Size() - _head);
			_head = 0;
		}
	}

	uint GetReserve() const { return _v.GetReserve(); }
	void Reserve(uint new_size) { AutoCompact(); _v.Reserve(new_size + _head); }

	void AutoCompact() { if (_v.GetReserve() >= 64 && _head > _v.Size() / 2)  Compact(); }

	uint Size() const { return _v.Size() - _head; }
	void Clear() { _v.Clear(); _head = 0; }
	bool Empty() const { return !Size(); }

	T& Front() { assert(_v.Size() > _head); return _v[_head]; }
	const T& Front() const { assert(_v.Size() > _head); return _v[_head]; }
	
	T& Back() { return _v.Back(); }
	const T& Back() const { return _v.Back(); }
	
	T& operator[](uint arg) { return _v[_head + arg]; }
	const T& operator[](uint arg) const { return _v[_head + arg]; }

	T* operator+(uint arg) { return _v + (_head + arg); }
	const T* operator+(uint arg) const { return _v + (_head + arg); }
	
	// Note: we only compact on insertion, never on erase.  This is
	// because the intended use for this structure is for a task run
	// queue.  We don't want a task to start right off the bat with a
	// housekeeping chore since that would add a random latency.
	// Instead we amortize all the housekeeping during insertion.
	
	uint Insert(const T& arg) { 
		uint pos = _v.FindGreaterThan(arg, _head);
		assert(pos >= _head);
		pos -= _head;
		if (!pos && _head) { _v[--_head] = arg; return _head; }

		_v.InsertInternal(pos + _head) = arg;
		AutoCompact();
		return pos;
	}

	void Erase(uint pos, uint num = 1) {
		if (!pos) { assert(_head + num <= _v.Size()); _head += num; }
		else _v.Erase(_head + pos, num);
	}

	void PopFront() { Erase(0, 1); }
	void PopBack() { Erase(Size() - 1, 1); }

	uint Find(const T& arg) {
		const uint pos = _v.Find(arg, _head);
		if (pos == NOT_FOUND)
			return pos;

		assert(pos >= _head);
		return pos - _head;
	}
};

#endif // __ODEQUE_H__
