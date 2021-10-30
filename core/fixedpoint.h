// Copyright (c) 2021 Jan Brittenson
// See LICENSE for details.

#ifndef __FIXEDPOINT_H__
#define __FIXEDPOINT_H__

template <typename T, uint FBITS, typename T2>
class FixedPoint {
    T _v;
public:
    FixedPoint(T n) : _v(n << FBITS) { }
    FixedPoint(int n) : _v(n << FBITS) { }

    FixedPoint(const FixedPoint& arg) : _v(arg._v) { }
    FixedPoint& operator=(const FixedPoint& rhs) {
        if (&rhs != this)
            _v = rhs._v;

        return *this;
    }

    FixedPoint(const float& f) : _v(f * (1 << FBITS)) {
    }

    FixedPoint operator+(const FixedPoint& rhs) {
        return FixedPoint(_v + rhs._v);
    }

    FixedPoint operator-(const FixedPoint& rhs) {
        return FixedPoint(_v - rhs._v);
    }

    FixedPoint operator*(const FixedPoint& rhs) {
        T2 tmp = (T2)_v * rhs._v;
        return FixedPoint(tmp >> FBITS);
    }

    FixedPoint operator/(const FixedPoint& rhs) {
        return FixedPoint((T)(((T2)_v << FBITS) / rhs._v));
    }

    FixedPoint& operator+=(const FixedPoint& rhs) {
        _v += rhs._v;
        return *this;
    }

    FixedPoint& operator-=(const FixedPoint& rhs) {
        _v -= rhs._v;
        return *this;
    }

    FixedPoint& operator*=(const FixedPoint& rhs) {
        *this = *this * rhs;
        return *this;
    }

    FixedPoint& operator/=(const FixedPoint& rhs) {
        *this = *this / rhs;
        return *this;
    }

    T Int() const {
        return _v >> FBITS;
    }

    FixedPoint abs() const {
        return FixedPoint(_v < 0 ? -_v : _v);
    }

    FixedPoint floor() const {
        return FixedPoint(Int());
    }

    FixedPoint ceil() const {
        return _v != Int() ? Int() + 1 : floor();
    }

    bool operator==(const FixedPoint& rhs) const {
        return _v == rhs._v;
    }

    bool operator<(const FixedPoint& rhs) const {
        return _v <= rhs._v;
    }

    bool operator>(const FixedPoint& rhs) const {
        return _v > rhs._v;
    }

    operator bool() const {
        return _v != 0;
    }

    String ToString() const;
};

typedef FixedPoint<uint16_t, 14, uint32_t> Q2_14;
typedef FixedPoint<uint16_t, 12, uint32_t> Q4_12;
typedef FixedPoint<uint16_t, 10, uint32_t> Q6_10;
typedef FixedPoint<uint16_t, 8, uint32_t> Q8_8;

typedef FixedPoint<uint32_t, 28, uint64_t> Q4_28;
typedef FixedPoint<uint32_t, 24, uint64_t> Q8_24;
typedef FixedPoint<uint32_t, 20, uint64_t> Q12_20;

#endif // __FIXEDPOINT_H__
