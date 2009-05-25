#ifndef __MEM_H__
#define __MEM_H__

void __noalias *xmalloc(uint size);
void __noalias *xrealloc(void *old, uint size);
void __noalias *xmemdup(const void* block, uint size);
void xxfree(void* ptr);

inline void xfree(void* ptr) { xxfree(ptr); }

#define STR(X)  ((const uchar*)(X))

struct MemStats {
	uint num_malloc;
	uint num_realloc;
	uint num_free;
};

const MemStats& xmemstats();

inline void* operator new(size_t size) { return xmalloc(size); }
inline void operator delete(void *ptr) { xfree(ptr); }

#ifdef ENETCORE
//  libc replacements

extern "C" {
// These need C linkage so gcc can use them from initializers
void* memset(void* b, int c, size_t n);
void* memcpy(void* __restrict s1, const void* __restrict s2, size_t n);
}

#ifdef __arm__
__force_inline void byte_copy_ascending(void* s1, const void* s2, uint n)
{
	asm volatile("1: subs %2, %2, #1; ldrgeb r2, [%1], #1; strgeb r2, [%0], #1; bgt 1b"
				 : : "r" (s1), "r" (s2), "r" (n) : "r2", "memory", "cc");
}


__force_inline void* memset(void* b, int c, size_t n) {
	asm volatile("1: strb %2, [%0], #1; subs %1, %1, #1; bne 1b"
				 :  : "r" (b), "r" (n), "r" (c) : "memory", "cc");
	return b;
}


__force_inline char* strcpy(char* __restrict dest, const char* __restrict src) {
	asm volatile("1: ldrb r2, [%1], #1; strb r2, [%1], #1; cmp r2, #0; bne 1b"
				 : : "r" (dest), "r" (src) : "r2", "memory", "cc");
	return dest;
}

__force_inline char* strncpy(char* __restrict dest, const char* __restrict src, size_t n) {
	asm volatile("1: subs %2, %2, #1;"
				 "ldrgeb r2, [%1], #1; strgeb r2, [%1], #1;"
				 "tstge r2, r2; bne 1b"
				 : : "r" (dest), "r" (src), "r" (n) : "r2", "memory", "cc");
	return dest;
}

__force_inline size_t strlen(const char* s) {
	size_t len;
	asm volatile("mov %0, #0;"
				 "1: ldrb r2, [%1], #1; cmp r2, #0; addne %0, %0, #1; bne 1b"
				 : "=&r" (len) : "r" (s) : "r2", "cc");
	return len;
}
#else
void* memset(void* b, int c, size_t n);
char* strcpy(char* __restrict dest, const char* __restrict src);
char* strncpy(char* __restrict dest, const char* __restrict src, size_t n);
size_t strlen(const char* s);
#endif

__force_inline int toupper(int c) {
	return c >= 'a' && c <= 'z' ? c - ('a' - 'A') : c;
}


char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
char* strstr(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
int strcasecmp(const char* s1, const char* s2);
int strncasecmp(const char* s1, const char* s2, size_t n);
int strcmp(const char* s1, const char* s2);
int memcmp(const void* s1, const void* s2, size_t n);
void* memmove(void* s1, const void* s2, size_t n);
#endif	// ENETCORE


uchar* xstrdup(const uchar* s);
uchar* __noalias xstrndup(const uchar* s, uint num);
uchar* xmemtostr(const void* block, uint size);

__force_inline uint xstrlen(const uchar* s) { return ::strlen((const char*)s); }
inline int xstrncasecmp(const uchar* a, const uchar* b, uint len) {
	return ::strncasecmp((const char*)a, (const char*)b, len);
}
__force_inline int xstrcasecmp(const uchar* a, const uchar* b) {
	return ::strcasecmp((const char*)a, (const char*)b);
}
__force_inline int xstrcmp(const uchar* a, const uchar* b) {
	return ::strcmp((const char*)a, (const char*)b);
}

// Note that s is const, but return value is non-const.  Just like strchr().
__force_inline uchar* xstrchr(const uchar* s, uchar c) { return (uchar*)::strchr((const char*)s, c); }
__force_inline uchar* xstrrchr(const uchar* s, uchar c) { return (uchar*)::strrchr((const char*)s, c); }

__force_inline uchar* xstrstr(const uchar* s1, const uchar* s2) {
	return (uchar*)::strstr((const char*)s1, (const char*)s2);
}

__force_inline uchar* xstrcpy(uchar* dest, const uchar* src) {
	return (uchar*)::strcpy((char*)dest, (const char*)src);
}
__force_inline uchar* xstrncpy(uchar* dest, const uchar* src, uint n) {
	return (uchar*)::strncpy((char*)dest, (const char*)src, n);
}


template <typename T> inline void move(T* dest, T* src, uint items) {
	memmove(dest, src, items * sizeof(T));
}


#if 0
// XXX put this and varius memcpy stuff in arm_asm.s?

__force_inline uint ReadLE16(const void* mem)
{
	uint tmp;
	asm volatile (" ldrb %0, [%1], #1;"
				  " ldrb r2, [%1];"
				  " or %0, r2, lsl #8";
				  : "=r" (tmp) : "r" (mem) : "r2" );
	return tmp;
}


__force_inline uint ReadLE32(const void* mem)
{
	uint tmp;
	asm volatile (" ldrb %0, [%1], #1;"
				  " ldrb r2, [%1], #1;"
				  " or %0, r2, lsl #8;"
				  " ldrb r2, [%1], #1;"
				  " or %0, r2, lsl #16;"
				  " ldrb r2, [%1];"
				  " or %0, r2, lsl #24"
				  : "=r" (tmp) : "r" (mem) : "r2" );
	return tmp;
}


__force_inline void StoreLE16(uint val, const void* mem)
{
	asm volatile (" strb %0, [%1], #1;"
				  " mov %0, %0, lsr #8;"
				  " strb %0, [%1]"
				  : : "r" (val), "r" (mem) );
}


__force_inline void StoreLE32(uint val, const void* mem)
{
	asm volatile (" strb %0, [%1], #1;"
				  " mov %0, %0, lsr #8;"
				  " strb %0, [%1], #1;"
				  " mov %0, %0, lsr #8;"
				  " strb %0, [%1], #1;"
				  " mov %0, %0, lsr #8;"
				  " strb %0, [%1]"
				  : : "r" (val), "r" (mem) );
}

#endif // 0

#endif // __MEM_H__