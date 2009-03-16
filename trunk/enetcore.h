#ifndef __ENETCORE_H__
#define __ENETCORE_H__

#define ENETCORE 1

#include <stdarg.h>
#define NULL 0

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;
typedef signed char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;

typedef uint32_t size_t;

typedef unsigned int uint;
#define uchar uint8_t

#include <new>

// Largest integer that will fit in a register
#if defined(x86_64) || defined(ppc64) || defined(sparc64)
typedef uint64_t native_uint_t;
typedef int64_t native_int_t;
#define NATIVE_BITS 64			// These are #defines so we can use them in cpp
#else
typedef uint32_t native_uint_t;
typedef int32_t native_int_t;
#define NATIVE_BITS 32
#endif

typedef native_int_t intptr_t;
typedef native_uint_t uintptr_t;
typedef intptr_t ptrdiff_t;

#define WEAK  __attribute__((weak))			// Weak symbol - can be redefined
#define NORETURN  __attribute__((noreturn)) // Function never returns
#define PURE __attribute__((pure)) // Indicates function has no meaningful side effects
#define VERYPURE __attribute__((const)) // Indicates function does not dereference global memory
#define CONSTRUCTOR  __attribute__((constructor)) // Static constructor, called before main
#define DESTRUCTOR  __attribute__((destructor)) // Static destructor, called after exit
#define NOALIAS  __attribute__((malloc)) // Return value is unalised pointer
#define NOINLINE  __attribute__((noinline))	// Do not inline

typedef uint32_t time_t;

#define USE_LITERALS

#include "hardware.h"
#include "config.h"
#include "mem.h"

#include "assert.h"

enum { NOT_FOUND = (uint)-1 };

typedef bool (*OrderFunc)(const void* a, const void* b);

#include "arithmetic.h"
#include "dlmalloc.h"
#include "vector.h"
#include "deque.h"
#include "trace.h"
#include "ovector.h"
#include "odeque.h"
#include "set.h"
#include "platform.h"
#include "freelist.h"
#include "time.h"
#include "pstring.h"

#endif // __ENETCORE_H__
