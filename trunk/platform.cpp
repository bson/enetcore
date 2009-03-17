#include "enetkit.h"
#include "platform.h"
#include "serial.h"


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
	assert(reserve <= _end - _start);
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

//void abort() { _lcd.WriteSync(STR("\xfe\xc0""ABORT")); fault(2); for (;;) ; }
void abort() { panic("ABORT"); }

#ifndef USE_ASM_MEMOPS
void* memset(void* b, int c, size_t n)
{
	uint8_t* p = (uint8_t*)b;
	while (n--)  *p++ = c;
	return b;
}


void* memcpy(void* __restrict s1, const void* __restrict s2, size_t n)
{
	uint8_t* p1 = (uint8_t*)s1;
	const uint8_t* p2 = (const uint8_t*)s2;
	while (n--)  *p1++ = *p2++;
	return s1;
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
#endif // USE_ASM_MEMOPS


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


int memcmp(const void* s1, const void* s2, size_t n) 
{
	const char* p1 = (const char*)s1;
	const char* p2 = (const char*)s2;

	while (n--) {
		if (*p1 > *p2) return 1;
		if (*p1 < *p2) return -1;
		++p1;
		++p2;
	}

	if (!n) return 0;
	if (*p1 < *p2) return -1;
	return 1;
}


void* memmove(void* s1, const void* s2, size_t n)
{
	if (s1 == s2) return s1;

	if (s1 < s2)  return memcpy(s1, s2, n);

	// s1 > s2
	asm volatile("add %0, %0, %2; add %1, %1, %2;"
				 "1: ldrb r2, [%1,#-1]!; strb r2, [%0,#-1]!; sub %2, %2, #1; bne 1b"
				 : : "r" (s1), "r" (s2), "r" (n) : "r2");

	return s1;
}


char* strchr(const char* s, int c)
{
	asm volatile("sub %0, %0, #1;"
				 "1: ldrb r2, [%0,#1]!; cmp r2, #0; cmpne r2, %1; bne 1b;"
				 "cmp r2, #0; moveq %0, #0"
				 : "=r" (s) : "0" (s), "r" (c) : "r2");
	return (char*)s;

#if 0
	while (*s && *s != c) s++;

	if (!*s) return NULL;

	return s;
#endif
}


char* strrchr(const char* s, int c)
{
	char* last;
	asm volatile("sub %1, %1, #1; mov %0, #0;"
				 "1: ldrb r2, [%1,#1]!; cmp r2, %2; moveq %0, %1;"
				 "cmp r2, %2; bne 1b"
				 : "=&r" (last) : "r" (s), "r" (c) : "r2");
	return last;
#if 0
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


// ABI stuff

extern "C" {
int __cxa_atexit(void (*func) (void*), void* arg, void* dso_handle);
int __cxa_pure_virtual();
}

int __cxa_atexit(void (*func) (void*), void* arg, void* dso_handle)
{
}

int __cxa_pure_virtual() { abort(); }

void*   __dso_handle = (void*) &__dso_handle;
