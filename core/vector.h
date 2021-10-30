// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "core/arithmetic.h"

// Simple vector
// Safe for use with POD only

template <typename T> class Vector {
	T* _mem;
	uint _alloc;
	uint _used;
	bool _autoresize;

	typedef Vector<T> Self;

public:
	Vector() : _mem(NULL), _alloc(0), _used(0), _autoresize(true) { }
	Vector(uint reserve) : _mem(NULL), _alloc(0), _used(0), _autoresize(true) {
		Reserve(reserve);
	}

	Self& assign(const Self& arg) {
		if (&arg == this)  return *this;

		xfree(_mem);

		if (arg._alloc) {
			_mem = (T*)xmemdup(arg._mem, sizeof (T) * arg._alloc);
			_alloc = arg._alloc;
			_used = arg._used;
		} else {
			_mem = NULL;
			_alloc = _used = 0;
		}
		return *this;
	}

	Vector(const Self& arg) { _mem = NULL; assign(arg); }
	Self& operator=(const Self& arg) { return assign(arg); }

	~Vector() { xfree(_mem); }

	void SetAutoResize(bool flag) { _autoresize = flag; }

	void FreeEntries(uint start = 0) { for (uint i = start; i < _used; ++i)  xfree(_mem[i]); }
	void DeleteEntries(uint start = 0) {
		for (uint i = start; i < _used; ++i)  delete _mem[i];
	}

	void DeleteObjects(uint start = 0) {
		for (uint i = start; i < _used; ++i) { if (_mem[i]) _mem[i]->Delete(); }
	}

	void SetMem(T* arg) { _mem = arg; }
	uint& used() { return _used; }
	uint& alloc() { return _alloc; }

	T* Grab(uint& used, uint& start, uint& alloc) {
		used = exch(_used, (uint)0);
        alloc = exch(_alloc, (uint)0);
		start = 0;
		return exch(_mem, (T*)NULL);
	}

	template <typename TT> Self& Take(TT& arg) {
		Clear();
		uint tmp;
		_mem = arg.Grab(_used, tmp, _alloc);
		return *this;
	}

	void Reserve(uint new_size) {
		assert(new_size);
		_mem = (T*)xrealloc(_mem, sizeof(T) * new_size);
		_alloc = new_size;
		_used = min(_used, _alloc);
	}

	uint GetReserve() const { return _alloc; }

	// Returns previous size (i.e. start of new additions)
	uint Grow(uint num) {
		_used += num;
		if (_used > _alloc) {
			if (!_autoresize) 
                panic("static container overflow");
			_alloc = _used + 32;
			_mem = (T*)xrealloc(_mem, sizeof(T) * _alloc);
		}
		return _used - num;
	}

	uint Headroom() const { return _alloc - _used; }
	void SetSize(uint arg) { assert_bounds(arg <= _alloc); _used = arg; }

	uint Size() const { return _used; }
	void Clear() {
		if (_autoresize) {
			xfree(exch<T*>(_mem, NULL));
			_alloc = 0;
		}
		_used = 0;
	}
	bool Empty() const { return _used == 0; }
	
	T& Front() { assert_bounds(_used); return _mem[0]; }
	const T& Front() const { assert_bounds(_used); return _mem[0]; }
	
	T& Back() { assert_bounds(_used); return _mem[_used-1]; }
	const T& Back() const { assert_bounds(_used); return _mem[_used-1]; }

	T& operator[](uint arg) { assert_bounds(arg < _used); return _mem[arg]; }
	const T& operator[](uint arg) const {
		assert_bounds(arg < _used);
		return _mem[arg];
	}
	
	T* operator+(uint arg) { assert_bounds(arg < _used); return _mem + arg; }
	const T* operator+(uint arg) const { assert_bounds(arg < _used); return _mem + arg; }

	T& Insert(uint pos, uint num = 1) {
		assert_bounds(pos <= _used);

		if (pos < _used) {
			Grow(num);				// _used += num
			move(_mem + pos + num, _mem + pos, _used - pos - num);
		} else {
			// Special case: insert at tail
			Grow(num);				// _used += num
		}

		return _mem[pos];
	}

	void PushFront(const T& arg) { Insert(0) = arg; }
	void PushBack(const T& arg) { Insert(_used) = arg; }

	// Append 0 terminated list; does not append 0 terminator.
	// Returns first item appended or Size() if arg list was empty.
	uint PushBack(const T* arg, int len = -1) {
		if (len == -1) {
			const T* p = arg;
			while (*p)
                ++p;
			len = p - arg;
		}

		if (!len) 
            return Size();

		const uint result = Grow(len);
		::memcpy(_mem + result, arg, sizeof (T) * len);
		return result;
	}

	void PushBack(const Self& arg) {
		if (!arg.Empty()) {
			const uint pos = Grow(arg.Size());
			::memcpy(_mem + pos, arg + 0, arg.Size());
		}
	}

	void Erase(uint pos, uint num = 1) {
		assert_bounds(pos < _used);

		num = min(num, _used - pos);
        _used -= num;
        if (_used)
            move<T>(_mem + pos, _mem + pos + num, _used - pos);
	}

	void PopFront() { Erase(0, 1); }
	void PopBack() { if (_used) --_used; }

	// Test if two T's are equal
	bool IsEqual(const T& a, const T& b) const { return a == b; }

	uint Find(const T& arg, uint start = 0) const {
		for (uint i = start; i < _used; ++i)
			if (IsEqual(_mem[i], arg))
				return i;

		return NOT_FOUND;
	}
};

#endif // __VECTOR_H__
