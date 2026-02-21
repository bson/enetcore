// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef __BITFIELD_H__
#define __BITFIELD_H__

#include <stdint.h>

#include "bits.h"


template <typename T = uint32_t>
class BitfieldT {
    const T _val;

public:
    BitfieldT<T>(const T& val = 0)
        : _val(val)
    { }

    BitfieldT<T>(const volatile T& val = 0)
        : _val(val)
    { }

    // Unbox
    const T& value() const { return _val; }
    operator T() const { return _val; }

    // Set a bit
    Bitfield<T> bit(int n) const { return Bitfield<T>(_val | BIT(n)); }

    // Clear a bit
    Bitfield<T> cbit(int n) const { return Bitfield<T>(_val & ~BIT(n)); }

    // Set a bit to a value
    Bitfield<T> bit(int n, bool v) const { return Bitfield<T>(_val | T(v ? BIT(n) : 0)); }
    Bitfield<T> bit(int n, int v) const { return Bitfield<T>(_val | T(v ? BIT(n) : 0)); }


    // Set a field to a value
    Bitfield<T> f(int bits, int bit0, int val) const {
        const unsigned int mask = (1 << bits) - 1;
        return Bitfield<T>((_val & ~(T(mask) << bit0)) | ((T(val) & T(mask)) << bit0));
    }

    BitfieldT<T>(const BitfieldT<T>&) = delete;
    BitfieldT<T>& operator=(const BitfieldT<T>&) = delete;
};

#endif // __BITFIELD_H__
