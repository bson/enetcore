#ifndef __COMPILER_H__
#define __COMPILER_H__

#ifndef __GNUC__
#error "Wrong compiler"
#endif

#include <stdarg.h>
#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned int size_t;

typedef uint32_t in_addr_t;

// Attributes
#define __used gnu::used
#define __weak gnu::weak
#define __pure gnu::pure // Indicates function has no meaningful side effects
#define __verypure gnu::const // Indicates function does not dereference global memory
#define __optimize gnu::optimize("O3") // Forces full function optimization
#define __constructor  gnu::constructor // Static constructor, called before main
#define __destructor  gnu::destructor // Static destructor, called after exit
#define __noalias  gnu::malloc         // Return value is unalised pointer
#define __noinline  gnu::noinline          // Do not inline
#define __finline  gnu::always_inline           // Force inline
#define __naked gnu::naked
#define __noinstrument gnu::no_instrument_function
#define __novtable

#define __section(SECT) gnu::section(SECT)

typedef uint32_t time_t;

#ifndef offsetof
#define offsetof(type, member)  __builtin_offsetof (type, member)
#endif

// Atomic ops
#define __atomic_inc(PTR)  \
    __sync_add_and_fetch((PTR), 1)

#define __atomic_dec(PTR)  \
    __sync_sub_and_fetch((PTR), 1)


#endif // __COMPILER_H__
