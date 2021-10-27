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
        return _v != Int() ? FixedPoint(Int() + 1) : floor();
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

    String ToString() const;
};

#endif // __FIXEDPOINT_H__
