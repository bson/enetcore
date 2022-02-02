// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __RING_H__
#define __RING_H__


// Ring buffer of static size. Size should be a power of two, but can technically
// be any integer value.  Designed purely for performance.

template <int N, typename T = uint8_t, typename size_type = uint>
class Ring {
    size_type _head;            // Index of first item - 1
    size_type _tail;            // Index of last item
    T _v[N];

    typedef Ring<N, T, size_type>  Self;

public:
    [[__optimize]] Ring() : _head(0), _tail(0) { }
    [[__optimize]] Ring(const Self& arg) {
        if (&arg != this) {
            memcpy(_v, arg, N);
            _head = arg._head;
            _tail = arg._tail;
        }
    }

    // Check that arg is valid index
    [[__optimize]] void BoundsCheck(uint arg) const { assert_bounds(arg < Size()); }
    [[__optimize]] void BoundsCheck(uint arg) const volatile { assert_bounds(arg < Size()); }
    [[__optimize]] void SpaceCheck(uint arg) const { assert_bounds(Headroom() >= arg); }
    [[__optimize]] void SpaceCheck(uint arg) const volatile { assert_bounds(Headroom() >= arg); }

    // Return amount of data in ring
    [[__optimize]] size_type Size() const {
        return _tail >= _head ? _tail - _head : N - _head + _tail;
    }
    [[__optimize]] size_type Size() const volatile {
        return _tail >= _head ? _tail - _head : N - _head + _tail;
    }

    // Return unused space
    [[__optimize]] size_type Headroom() const { return N - Size() - 1; }
    [[__optimize]] size_type Headroom() const volatile { return N - Size() - 1; }

    // Return amount of continuous buffer data, starting at _head
    [[__optimize]] size_type Continuous() const {
        if (_head <= _tail)
            return Size();

        // If head is at N-1, then there is 0 continuous after it.  So
        // report continuous at the beginning since the next item at
        // the head is in location 0.
        if (_head == N - 1)
            return _tail + 1;

        return N - _head - 1;
    }

    [[__optimize]] size_type Continuous() const volatile {
        if (_head <= _tail)
            return Size();

        // If head is at N-1, then there is 0 continuous after it.  So
        // report continuous at the beginning since the next item at
        // the head is in location 0.
        if (_head == N - 1)
            return _tail + 1;

        return N - _head - 1;
    }

    [[__optimize]] void Clear() { _head = _tail = 0; }
    [[__optimize]] void Clear() volatile { _head = _tail = 0; }
    [[__optimize]] bool Empty() const { return _head == _tail; }
    [[__optimize]] bool Empty() const volatile { return _head == _tail; }

    // Make _head <= _tail
    // Deprecated.  Don't use.
    void Flatten() {
        AssertNotInterrupt();

        if (_head <= _tail)
            return;

        // XXX do this in-place, with one or two memmoves.  We really
        // don't want to allocate a temporary buffer on the heap.
        extern void* xmalloc(size_t);
        T* tmp = xmalloc(N * sizeof (T));
        memcpy(tmp, _v, N * sizeof (T));
        const uint h = N - _head;
        memcpy(_v, tmp + _head, h * sizeof (T));
        memcpy(_v + h * sizeof (T), tmp, (_tail + 1) * sizeof (T));
        _tail = Size() - 1;
        _head = 0;
        free(tmp);
    }

    T& Pullup(uint arg) {
        BoundsCheck(arg);
        if (_head >= _tail || (N - _head >= arg))
            Flatten();

        return _v[_head];
    }
            
    [[__optimize]] T& Front() { BoundsCheck(0); return _v[(_head + 1) % N]; }
    [[__optimize]] const T& Front() const { BoundsCheck(0); return _v[(_head + 1) % N]; }
    [[__optimize]] T& Front() volatile { BoundsCheck(0); return _v[(_head + 1) % N]; }
    [[__optimize]] const T& Front() const volatile { BoundsCheck(0); return _v[(_head + 1) % N]; }

    [[__optimize]] T& Back() { BoundsCheck(0); return _v[_tail]; }
    [[__optimize]] const T& Back() const { BoundsCheck(0); return _v[_tail]; }
    [[__optimize]] T& Back() volatile { BoundsCheck(0); return _v[_tail]; }
    [[__optimize]] const T& Back() const volatile { BoundsCheck(0); return _v[_tail]; }

    [[__optimize]] const T* Buffer() const { BoundsCheck(0); return _v + ((_head + 1) % N); }
    [[__optimize]] const volatile T* Buffer() const volatile { BoundsCheck(0); return _v + ((_head + 1) % N); }

    [[__optimize]] void PushFront(const T& arg) {
        SpaceCheck(1);
        _head = (_head - 1) % N;
        _v[_head] = arg;
    }

    [[__optimize]] void PushFront(const T& arg) volatile {
        SpaceCheck(1);
        _head = (_head - 1) % N;
        _v[_head] = arg;
    }

    [[__optimize]] void PushFront(const T* arg, uint len = -1) {
        if (len == (uint)-1) {
            while (*arg)
                PushFront(*arg++);
        } else {
            SpaceCheck(len);
            while (len--)
                PushFront(*arg++);
        }
    }
    [[__optimize]] void PushFront(const T* arg, uint len = -1) volatile {
        if (len == (uint)-1) {
            while (*arg)
                PushFront(*arg++);
        } else {
            SpaceCheck(len);
            while (len--)
                PushFront(*arg++);
        }
    }

    [[__optimize]] void PushBack(const T arg) {
        SpaceCheck(1);
        _tail = (_tail + 1) % N;
        _v[_tail] = arg;
    }

    [[__optimize]] void PushBack(const T arg) volatile {
        SpaceCheck(1);
        _tail = (_tail + 1) % N;
        _v[_tail] = arg;
    }

    [[__optimize]] void PushBack(const T* arg, uint len = -1) {
        if (len == (uint)-1) {
            while (*arg)
                PushBack(*arg++);
        } else {
            SpaceCheck(len);
            while (len--)
                PushBack(*arg++);
        }
    }

    [[__optimize]] void PushBack(const T* arg, uint len = -1) volatile {
        if (len == (uint)-1) {
            while (*arg)
                PushBack(*arg++);
        } else {
            SpaceCheck(len);
            while (len--)
                PushBack(*arg++);
        }
    }

    [[__optimize]] T& operator[](uint arg) { BoundsCheck(arg); return _v[(_head + arg + 1) % N]; }
    [[__optimize]] const T& operator[](uint arg) const { BoundsCheck(arg); return _v[(_head + arg + 1) % N]; }
    [[__optimize]] T& operator[](uint arg) volatile { BoundsCheck(arg); return _v[(_head + arg + 1) % N]; }
    [[__optimize]] const T& operator[](uint arg) const volatile { BoundsCheck(arg); return _v[(_head + arg + 1) % N]; }
    
    [[__optimize]] T* operator+(uint arg) { BoundsCheck(arg); return _v + ((_head + arg + 1) % N); }
    [[__optimize]] const T* operator+(uint arg) const { BoundsCheck(arg); return _v + ((_head + arg + 1) % N); }
    [[__optimize]] T* operator+(uint arg) volatile { BoundsCheck(arg); return _v + ((_head + arg + 1) % N); }
    [[__optimize]] const T* operator+(uint arg) const volatile { BoundsCheck(arg); return _v + ((_head + arg + 1) % N); }

    [[__optimize]] T PopFront() {
        BoundsCheck(0);
        _head = (_head + 1) % N;
        return _v[_head];
    }

    [[__optimize]] T PopFront() volatile {
        BoundsCheck(0);
        _head = (_head + 1) % N;
        return _v[_head];
    }

    [[__optimize]] void PopFront(size_type n) {
        if (n) {
            BoundsCheck(n-1);
            _head = (_head + n) % N;
        }
    }

    [[__optimize]] void PopFront(size_type n) volatile {
        if (n) {
            BoundsCheck(n-1);
            _head = (_head + n) % N;
        }
    }

    [[__optimize]] T PopBack() {
        BoundsCheck(0);
        T val = _v[_tail];
        _tail = (_tail - 1) % N;
        return val;
    }

    [[__optimize]] T PopBack() volatile {
        BoundsCheck(0);
        T val = _v[_tail];
        _tail = (_tail - 1) % N;
        return val;
    }

    uint Find(const T& arg) {
        for (uint i = 0; i < Size(); ++i)
            if (operator[](i) == arg) return i;

        return NOT_FOUND;
    }

    void DeleteEntries() {
        AssertNotInterrupt();
        for (uint i = _head; i != _tail; i = (i + 1) % N)
            delete _v[i];
        Clear();
    }

};

#endif // __RING_H__
