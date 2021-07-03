// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __ARITHMETIC_H__
#define __ARITHMETIC_H__

#undef min
#undef max

template <typename T> inline T min(T a, T b) { return a < b ? a : b; }
template <typename T> inline T max(T a, T b) { return a > b ? a : b; }

template <typename T> inline T exch(T& a, const T& b) { const T tmp = a; a = b; return tmp; }

#undef IS_BIG_ENDIAN
#undef IS_LITTLE_ENDIAN

#if BYTE_ORDER == LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN 1
#else
#define IS_BIG_ENDIAN 1
#endif

#if IS_LITTLE_ENDIAN
#define Htons Swap16
#define Htonl Swap32
#define LE16(V) uint16_t(V)
#define LE32(V) uint32_t(V)
#else
#define Htons(V) uint16_t(V)
#define Htonl(V) uint32_t(V)
#define LE16 Swap16
#define LE32 Swap32
#endif

#define Ntohs Htons
#define Ntohl Htonl

#if defined(__arm__)
inline uint32_t Swap32(uint32_t val) {
    asm volatile ("rev %0, %0" : "+r" (val) : : );
    return val;
}
inline uint16_t Swap16(uint32_t val) {
    asm volatile ("rev16 %0, %0" : "+r" (val) : : );
    return val;
}

#elif defined(__i386__) || defined(__x86_64__)
inline uint32_t Swap32(uint32_t val)
{
	asm volatile ("bswapl %0" : "=r" (val) : "0" (val) : );
	return val;
}

inline uint16_t Swap16(uint16_t val)
{
	asm volatile ("xchgb %%ah, %%al" : "=a" (val) : "a" (val) : );
	return val;
}
#else
inline uint32_t Swap32(uint32_t val)
{
	return ((val & 0xff000000) >> 24) | ((val & 0xff0000) >> 8) | ((val & 0xff00) << 8) | ((val & 0xff) << 24);
}

inline uint16_t Swap16(uint16_t val)
{
	return ((val & 0xff00) >> 8) | ((val & 0xff) << 8);
}
#endif

#endif // __ARITHMETIC_H__
