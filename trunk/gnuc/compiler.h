#ifndef __COMPILER_H__
#define __COMPILER_H__

#ifndef __GNUC__
#error "Wrong compiler"
#endif

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

typedef uint32_t in_addr_t;

#define __weak  __attribute__((weak))			// Weak symbol - can be redefined
#define __noreturn  __attribute__((noreturn)) // Function never returns
#define __pure __attribute__((pure)) // Indicates function has no meaningful side effects
#define __verypure __attribute__((const)) // Indicates function does not dereference global memory
#define __constructor  __attribute__((constructor)) // Static constructor, called before main
#define __destructor  __attribute__((destructor)) // Static destructor, called after exit
#define __noalias  __attribute__((malloc)) // Return value is unalised pointer
#define __noinline  __attribute__((noinline))	// Do not inline
#define __force_inline inline __attribute__((always_inline))
#define __naked __attribute__((naked))
#define __noinstrument __attribute__((no_instrument_function))

#define __novtable
#define __packed __attribute__((packed))

#define __section(SECT) __attribute__((section(SECT)))

typedef uint32_t time_t;

#endif // __COMPILER_H__
