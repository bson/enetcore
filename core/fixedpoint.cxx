#include "enetkit.h"
#include "fixedpoint.h"


template <typename T, uint FBITS, typename T2>
String FixedPoint<T, FBITS, T2>::ToString() const {
    T v = _v;
    uchar fmt[32];
    uchar *p = fmt;
    if (v < 0) {
        *p++ = '-';
        v = -v;
    }
    *p++ = '%';
    *p++ = 'u';
    *p++ = '.';
    auto x = *this;
    T ip = x.Int();
    x -= x.Int();
    do {
        x *= 10;
        *p++ = '0' + x.Int();
        x -= x.Int();
    } while (x);

    *p++ = 0;

    return String::Format(fmt, ip);
}
