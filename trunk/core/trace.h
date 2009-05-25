#ifndef __TRACE_H__
#define __TRACE_H__

#include "util.h"


// Console message
void console(const char* fmt, ...);

// The logic here is such that:
//  - DEBUG defaults to trace, but a non-trace DEBUG can be built using NTRACE
//  - production defaults to no trace, but tracing can be enabled by defining TRACE
//  - After inclusion of this header file, TRACE reflects whether tracing is enabled

#if defined(DEBUG) && !defined(NTRACE) && !defined(TRACE)
#define TRACE
#endif

#if defined(TRACE)
void DMSG(const char* fmt, ...);
#else
inline void NDMSG(const char* fmt, ...) { }
#define DMSG NDMSG
#endif

#endif // __TRACE_H__
