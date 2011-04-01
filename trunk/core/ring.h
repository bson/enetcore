#ifndef __RING_H__
#define __RING_H__


// Ring buffer of static size. Size should be a power of two, but can technically
// be any integer value.  Designed purely for performance.

template <int N, typename size_type = uint, typename T = uint8_t>
class Ring {
    size_type _head;            // Index of first item
    size_type _tail;            // Index of last item (*not* last + 1!)
    T _v[N];

    typedef Ring<N, size_type, T>  Self;

public:
    Ring() : _head(0), _tail(0) { }
    Ring(const Self& arg) {
        if (&arg != this) {
            memcpy(_v, arg, N);
            _head = arg._head;
            _tail = arg._tail;
        }
    }
                
    // Check that arg is valid index
    void BoundsCheck(uint arg) const { assert_bounds(arg < Size()); }
    void SpaceCheck(uint arg) const { assert_bounds(Headroom() >= arg); }

    size_type Size() const {
        return _tail > _head ? _tail - _head : N - _tail + _head;
    }

    size_type Headroom() const { return N - Size(); }

    void Clear() { _head = _tail = 0; }
    bool Empty() const { return _head == _tail; }

    // Make _head <= _tail

    void Flatten() {
        AssertNotInterrupt();

        if (_head <= _tail) return;

        // Is there a way to do this in place, with one or two passes?  It would
        // be nice not to have to allocate a temporary buffer on the heap.  Not
        // to mention it can't be done in an interrupt context.
        T* tmp = malloc(N * sizeof (T));
        assert_alloc(tmp);
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
            
    T& Front() { BoundsCheck(0); return _v[_head]; }
    const T& Front() const { BoundsCheck(0); return _v[_head]; }

    T& Back() { BoundsCheck(0); return _v[_tail - 1]; }
    const T& Back() const { BoundsCheck(0); return _v[_tail - 1]; }

    void PushFront(const T& arg) {
        SpaceCheck(1);
        _head = (_head - 1) % N;
        _v[_head] = arg;
    }

    void PushFront(const T* arg, uint len = -1) {
        if (len == (uint)-1) {
            while (*arg)  PushFront(*arg++);
        } else {
            SpaceCheck(len);
            while (len--) PushFront(*arg++);
        }
    }

    void PushBack(const T& arg) {
        SpaceCheck(1);
        _tail = (_tail + 1) % N;
        _v[_tail] = arg;
    }

    void PushBack(const T* arg, uint len = -1) {
        if (len == (uint)-1) {
            while (*arg)  PushBack(*arg++);
        } else {
            SpaceCheck(len);
            while (len--) PushBack(*arg++);
        }
    }

    T& operator[](uint arg) { BoundsCheck(arg); return _v[(_head + arg) % N]; }
    const T& operator[](uint arg) const { BoundsCheck(arg); return _v[(_head + arg) % N]; }
    
    T* operator+(uint arg) { BoundsCheck(arg); return _v + ((_head + arg) % N); }
    const T* operator+(uint arg) const { BoundsCheck(arg); return _v + ((_head + arg) % N); }

    void PopFront() {
        BoundsCheck(0);
        _head = (_head + 1) % N;
    }

    void PopBack() {
        BoundsCheck(0);
        _tail = (_tail - 1) % N;
    }

    uint Find(const T& arg) {
        for (uint i = 0; i < Size(); ++i)
            if (operator[](i) == arg) return i;

        return NOT_FOUND;
    }

    void DeleteEntries() {
        for (uint i = _head; i != _tail; i = (i + 1) % N)
            delete _v[i];
    }

};

#endif // __RING_H__
