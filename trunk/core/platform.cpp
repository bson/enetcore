#include "enetkit.h"
#include "platform.h"


namespace Platform {

Region _malloc_region(MALLOC_REGION_SIZE, MALLOC_REGION_START);
Region _data_region(DATA_REGION_SIZE, DATA_REGION_START);
Region _stack_region(STACK_REGION_SIZE, STACK_REGION_START);
Region _text_region(TEXT_REGION_SIZE, TEXT_REGION_START);
Region _xflash_region(XFLASH_REGION_SIZE, XFLASH_REGION_START);


Region::Region() { }

void Region::Init(uint size, uint8_t* start)
{
	_lock.AssertLocked();

	_reserve = 0;

	if (start) {
		_next = start;
		_end = _start + size;
		return;
	}
	
	abort();
}


void* Region::GetMem(int amount)
{
	Spinlock::Scoped L(_lock);

	if (!amount) return _next;

	if (amount > 0) {
		if (_next + amount >= _end - _reserve)  return (void*)-1;

//		::memset(_next, 0, amount);

		return exch<uint8_t*>(_next, _next + amount);
	}

	// No point returning - just a waste of cycles since the region is fixed size
	abort();
}


void Region::SetReserve(uint reserve)
{
	assert(reserve <= (uint)(_end - _start));
	Spinlock::Scoped L(_lock);
	_reserve = reserve;
}


struct mallinfo {
	size_t arena;    /* non-mmapped space allocated from system */
	size_t ordblks;  /* number of free chunks */
	size_t smblks;   /* always 0 */
	size_t hblks;    /* always 0 */
	size_t hblkhd;   /* space in mmapped regions */
	size_t usmblks;  /* maximum total allocated space */
	size_t fsmblks;  /* always 0 */
	size_t uordblks; /* total allocated space */
	size_t fordblks; /* total free space */
	size_t keepcost; /* releasable (via malloc_trim) space */
};

extern "C" { extern struct mallinfo mallinfo(); }

void DebugMallocMsg()
{
#ifdef DEBUG
	struct mallinfo m = mallinfo();

	DMSG("Mem: malloc in use: %u/%u bytes, %u available (%u%%), %u free chunks",
		 m.uordblks, m.arena, m.fordblks + _malloc_region.GetFreeMem(), (m.fordblks*100/m.arena), m.ordblks);
#endif
}


uint GetFreeMem()
{
	struct mallinfo m = mallinfo();
	return m.fordblks + _malloc_region.GetFreeMem();
}


} // namespace Platform

// Runtime compat stuff that needs to go in global namespace

#if !defined(__arm__)
void* memset(void* b, int c, size_t n)
{
	uint8_t* p = (uint8_t*)b;
	while (n--)  *p++ = c;
	return b;
}


char* strcpy(char* __restrict dest, const char* __restrict src)
{
	for (char *tmp = dest; (*tmp++ = *src++); ) continue;
	return dest;
}


char* strncpy(char* __restrict dest, const char* __restrict src, size_t n)
{
	for (char *tmp = dest; n-- && (*tmp++ = *src++); ) continue;
	return dest;
}


size_t strlen(const char* s)
{
	size_t n = 0;
	while (*s++) ++n;
	return n;
}
#endif


int strcmp(const char* s1, const char* s2) 
{
	while (*s1 && *s2) {
		if (*s1 > *s2) return 1;
		if (*s1 < *s2) return -1;
		++s1;
		++s2;
	}
	if (!*s1 && !*s2) return 0;
	if (!*s1) return -1;
	return 1;
}


int strncmp(const char* s1, const char* s2, size_t n) 
{
	while (n-- && *s1 && *s2) {
		if (*s1 > *s2) return 1;
		if (*s1 < *s2) return -1;
		++s1;
		++s2;
	}
	if (!n || (!*s1 && !*s2)) return 0;
	if (!*s1) return -1;
	return 1;
}


int strcasecmp(const char* s1, const char* s2) 
{
	while (*s1 && *s2) {
		const int c1 = toupper(*s1);
		const int c2 = toupper(*s2);

		if (c1 > c2) return 1;
		if (c1 < c2) return -1;
		++s1;
		++s2;
	}
	if (!*s1 && !*s2) return 0;
	if (!*s1) return -1;
	return 1;
}


int strncasecmp(const char* s1, const char* s2, size_t n) 
{
	while (n-- && *s1 && *s2) {
		const int c1 = toupper(*s1);
		const int c2 = toupper(*s2);

		if (c1 > c2) return 1;
		if (c1 < c2) return -1;
		++s1;
		++s2;
	}
	if (!n || (!*s1 && !*s2)) return 0;
	if (!*s1) return -1;
	return 1;
}


int memcmp(const void* __restrict s1, const void* __restrict s2, size_t n) 
{
	const char* p1 = (const char*)s1;
	const char* p2 = (const char*)s2;

	while (n) {
		if (*p1 > *p2) return 1;
		if (*p1 < *p2) return -1;
		++p1;
		++p2;
		--n;
	}

	if (!n) return 0;
	if (*p1 < *p2) return -1;
	return 1;
}



void* memcpy(void* s1, const void* s2, size_t n)
{
	if (s1 == s2 || !n) return s1;

	uint8_t* dst = (uint8_t*)s1;
	const uint8_t* src = (const uint8_t*)s2;

#if defined(__arm__)
	uint align1 = (uintptr_t)dst & 3;
	uint align2 = (uintptr_t)src & 3;

	if (align1 && align1 == align2) {
		// Equal misalignment - bring to alignment
		align1 = 4-align1;
		byte_copy_ascending(dst, src, align1);
		dst += align1;
		src += align1;
		n -= align1;
		align1 = 0;
		align2 = 0;
	}

	if (!align1 && !align2) {
		// 32-bit aligned operands
		asm volatile("1: subs %2, %2, #4; ldrge r2, [%1], #4; strge r2, [%0], #4; bgt 1b;"
					 : : "r" (dst), "r" (src), "r" (n)
					 : "r2", "memory", "cc");
		dst += (n & ~3);
		src += (n & ~3);
		n &= 3;
	}

	// Byte copy remainder
	byte_copy_ascending(dst, src, n);
#else
	// Trivial reference implementation
	while (n--) *dst++ = *src++;
#endif
	return s1;
}


// Copy descending
void* memcpyd(void* s1, const void* s2, size_t n)
{
	if (s1 == s2 || !n) return s1;

#if defined(__arm__)
	if (!((uintptr_t)s1 & 3) && !((uintptr_t)s2 & 3)) {
		// 32-bit aligned operands.
		asm volatile("   add %0, %0, %2; add %1, %1, %2;"
					 "1: ldrb r2, [%1, #-1]!; strb r2, [%0, #-1]!;"
					 "   sub %2, %2, #1; tst %2, #3; bne 1b;"
					 "2: ldr r2, [%1, #-4]!; str r2, [%0, #-4]!;"
					 "   subs %2, %2, #4; bne 2b"
					 : : "r" (s1), "r" (s2), "r" (n) : "r2", "cc", "memory");
	} else {
		asm volatile("add %0, %0, %2; add %1, %1, %2;"
					 "1: ldrb r2, [%1,#-1]!; strb r2, [%0,#-1]!; subs %2, %2, #1; bne 1b"
					 : : "r" (s1), "r" (s2), "r" (n) : "r2", "cc", "memory");
	}
#else
	// Trivial reference implementation
	uint8_t* dst = (uint8_t*)s1 + n;
	const uint8_t* src = (const uint8_t*)s2 + n;
	while (n--) *--dst = *--src;
#endif

	return s1;
}


void* memmove(void* s1, const void* s2, size_t n)
{
	if (s1 < s2)  return memcpy(s1, s2, n);

	// s1 > s2
	return memcpyd(s1, s2, n);
}


char* strchr(const char* s, int c)
{
#ifdef __arm__
	asm volatile("sub %0, %0, #1;"
				 "1: ldrb r2, [%0,#1]!; cmp r2, #0; cmpne r2, %1; bne 1b;"
				 "cmp r2, #0; moveq %0, #0"
				 : "=r" (s) : "0" (s), "r" (c) : "r2", "cc");
	return (char*)s;
#else
	while (*s && *s != c) s++;

	if (!*s) return NULL;

	return s;
#endif
}


char* strrchr(const char* s, int c)
{
#ifdef __arm__
	char* last;
	asm volatile("sub %1, %1, #1; mov %0, #0;"
				 "1: ldrb r2, [%1,#1]!; cmp r2, %2; moveq %0, %1;"
				 "cmp r2, %2; bne 1b"
				 : "=&r" (last) : "r" (s), "r" (c) : "r2", "cc");
	return last;
#else
	const char* last = NULL;

	while (*s) {
		if (*s == c) last = s;
		++s;
	}

	return last;
#endif
}


char* strstr(const char* s1, const char* s2)
{
	if (!*s2) return NULL;

	const uint s2len = strlen(s2);

	const char* s = s1;
	while ((s = strchr(s, *s2)) && memcmp(s, s2, s2len) != 0)
		++s;

	return (char*)s;
}


// For gcc
#ifdef USE_ASM_MEMOPS
#undef memset

void* memset(void* b, int c, size_t n) { return xxmemset(b, c, n); }

#endif

bool isspace(char c) { return c && c <= ' '; }

template <typename T> T strto(const char* __restrict s, char **__restrict endptr, int base)
{
	assert(s);
	assert(base <= 10 || base == 16);

	// Ignore leading whitespace (includes control chars but not NUL)
	while (isspace(*s)) ++s;

	// - or +, but not both
	const bool neg = (*s == '-');
	if (*s == '+') ++s;
	if (neg) ++s;

	// Base 0 or 16 has an optional 0x prefix
	if ((!base || base == 16) && (s[0] == '0' && toupper(s[1]) == 'X')) {
		s += 2;
		base = 16;
	}

	// Without a prefix, base 0 defaults to 10
	if (!base) base = 10;

	// For base 2 and 16, we shift.  We could shift for other power of 2
	// bases as well, but they're so rare there's no point adding code for it.
	const int8_t shiftbase = (base == 2 ? 1 : base == 16 ? 4 : 0);

	const char maxdigit = base >= 10 ? '9' : base - 1;

	T result = 0;
	for (;;) {
		const char c = toupper(*s);

		if ((c >= '0' && c <= maxdigit) || (base == 16 && c >= 'A' && c <= 'F')) {
			if (shiftbase)
				result <<= shiftbase;
			else
				result *= base;

			if (c >= 'A' && c <= 'F')
				result += (c - 'A') + 10;
			else 
				result += c - '0';

			++s;
		} else {
			if (endptr) *endptr = (char*)s;
			return neg ? -result : result;
		}
	}
}


long strtol(const char* __restrict str, char **__restrict endptr, int base)
{
#ifdef LP64
	// On a 64-bit platform 32/64 bit are the same speed, so no need for both
	return strto<int64_t>(str, endptr, base);
#else
	// On a 32-bit platform a 32-bit numeric parse is orders of magnitude faster,
	// so in a nod to performance we use a separate 32-bit version.
	return strto<long>(str, endptr, base);
#endif
}


int64_t strtoll(const char* __restrict str, char **__restrict endptr, int base)
{
	return strto<int64_t>(str, endptr, base);
}


int atoi(const char* s) { return strtol(s, NULL, 10); }


in_addr_t inet_addr(const char* a)
{
	in_addr_t ip;
	uint8_t* ptr = (uint8_t*)&ip;

	while (a) {
		char* end;
		const int octet = strtol(a, &end, 10);
		if (end == a) return 0;
		*ptr++ = octet;
		a = end;
	}
	return ip;
}


#undef abort
void abort() { panic("ABORT"); }


// ABI stuff

extern "C" {
int __cxa_atexit(void (*func) (void*), void* arg, void* dso_handle);
int __cxa_pure_virtual();
}

int __cxa_atexit(void (*func) (void*), void* arg, void* dso_handle)
{
	return 0;
}

int __cxa_pure_virtual() { abort(); return 0; }

void*   __dso_handle = (void*) &__dso_handle;
