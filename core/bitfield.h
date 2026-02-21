// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __BITFIELD_H__
#define __BITFIELD_H__

#include "bits.h"


template <typename T>
class Bitfield {
    const T _val;

public:
    Bitfield<T>(const T& val = 0)
        : _val(val)
    { }

    Bitfield<T>(const volatile T& val = 0)
        : _val(val)
    { }

    // Unbox
    const T& value() const { return _val; }

    // Set a bit
    Bitfield<T> bit(int n) const { return Bitfield<T>(_val | BIT(n)); }

    // Set a bit to a value
    Bitfield<T> bit(int n, bool v) const { return Bitfield<T>(_val | (v ? BIT(n) : 0)); }
    Bitfield<T> bit(int n, int v) const { return Bitfield<T>(_val | (v ? BIT(n) : 0)); }

    // Set a field to a value
    Bitfield<T> f(int bits, int bit0, int val) const {
        const unsigned int mask = (1 << bits) - 1;
        return Bitfield<T>((_val & ~(mask << bit0)) | ((val & mask) << bit0));
    }

    Bitfield<T>(const Bitfield<T>&) = delete;
};

#endif // __BITFIELD_H__
