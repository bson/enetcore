#include "enetkit.h"
#include "fixedpoint.h"


template <typename T, uint FBITS, typename T2>
String FixedPoint<T, FBITS, T2>::ToString() const {
    uchar fmt[32];
    uchar *p = fmt;
    T v = _v;
    *p++ = '%';
    *p++ = 'd';
    *p++ = '.';
    T ip = Int();
    FixedPoint x = *this;
    x -= ip;
    do {
        x *= 10;
        *p++ = '0' + x.Int();
        x -= x.Int();
    } while (x && p < fmt + sizeof fmt - 2);

    *p = 0;
    return String::Format(fmt, ip);
}
