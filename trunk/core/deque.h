#ifndef __DEQUE_H__
#define __DEQUE_H__

// Functionally a deque, but doesn't adher to the STL's complexity contract
// It's well suited for pushing at the back and popping from the front, while
// poorly suited for pushing at the front and popping at the back.
// Use for standard layout types only.

template <typename T> class Deque {
	Vector<T> _v;
	uint _head;					// Start of "ring"
	bool _autocompact;

	typedef Deque<T> Self;

public:
	Deque() { _head = 0; _autocompact = true; }
	Deque(uint reserve) : _v(reserve) { _head = 0; _autocompact = true; }
	Deque(const Self& arg) : _v(arg._v) { _head = arg._head; _autocompact = true; }
	Deque(const Vector<T>& arg) : _v(arg) { _head = 0; _autocompact = true; }

	virtual ~Deque() { };

	uint Headroom() const { return _v.Headroom() + _head; }

	void Compact() {
		if (_head) {
			_v.Erase(0, _head);
			_head = 0;
		}
	}

	template <typename TT> void Take(TT& arg) {
		Clear();
		_v.setmem(arg.Grab(_v.used(), _head, _v.alloc()));
	}

	T* Grab(uint& used, uint& start, uint& alloc) {
		Compact();
		assert(!_head);
		return _v.Grab(used, start, alloc);
	}

	void AutoCompact() {
		if (!_autocompact) return;
		if (_v.Size() > 32 && _head > _v.Size() / 2)  Compact();
	}

	void SetAutoResize(bool flag) { _v.SetAutoResize(flag); }
	void SetAutoCompact(bool arg) { _autocompact = arg; }

	void Reserve(uint new_size) { AutoCompact(); _v.Reserve(new_size + _head); }
	uint GetReserve() const { return _v.GetReserve(); }
	uint Grow(uint num) { return _v.Grow(num) - _head; }
		
	uint Size() const { return _v.Size() - _head; }
	void Clear() { _v.Clear(); _head = 0; }
	bool Empty() const { return _head == _v.Size(); }

	void SetMem(T* arg, uint alloc) {
		_v.SetMem(arg); _v.alloc() = alloc; _head = 0; SetSize(0);
	}
	void SetSize(uint arg) { _v.SetSize(_head + arg); }

	void SetHead(uint arg) { _head = arg; }
	void SetTail(uint arg) { _v.SetSize(arg); }

	T& Front() { assert_bounds(_v.Size() > _head); return _v[_head]; }
	const T& Front() const { assert_bounds(_v.Size() > _head); return _v[_head]; }
	
	T& Back() { return _v.Back(); }
	const T& Back() const { return _v.Back(); }
	
	T& operator[](uint arg) { return _v[_head + arg]; }
	const T& operator[](uint arg) const { return _v[_head + arg]; }

	T* operator+(uint arg) { return _v + (_head + arg); }
	const T* operator+(uint arg) const { return _v + (_head + arg); }
	
	T& Insert(uint pos, uint num = 1) {
		// Special case: insert at head; see if we can simply step _head back.
		// If we were smarter here we'd shift in the direction resulting in the
		// smaller move. (I.e. towards head or tail.)
		if (!pos && _head >= num) { return _v[(_head -= num)]; }

		AutoCompact();
		return _v.Insert(_head + pos, num);
	}

	void PushFront(const T& arg) { Insert(0) = arg; }
	void PushBack(const T& arg) { Insert(Size()) = arg; }
	uint PushBack(const T* arg, int len = -1) {
		const uint pos = _v.PushBack(arg, len); AutoCompact(); return pos;
	}
	void PushFront(const Self& arg) {
		if (!arg.Empty())
			::memcpy(&Insert(0, arg.Size()), arg + 0, arg.Size() * sizeof(T));
	}

	void Erase(uint pos, uint num = 1) {
		if (!pos) { assert_bounds(_head + num <= _v.Size()); _head += num; }
		else _v.Erase(_head + pos, num);
		AutoCompact();
	}

	void PopFront() { Erase(0, 1); }
	void PopBack() { _v.PopBack(); }

	uint Find(const T& arg) {
		const uint pos = _v.Find(arg, _head);
		if (pos == NOT_FOUND)  return pos;
		assert_bounds(pos >= _head);
		return pos - _head;
	}

	void DeleteEntries() { _v.DeleteEntries(_head); }
};

#endif // __DEQUE_H__
