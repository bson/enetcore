#ifndef __ENETCORE_H__
#define __ENETCORE_H__

#define ENETCORE 1

#include "compiler.h"

#define USE_LITERALS

#include "hardware.h"
#include "spinlock.h"
#include "config.h"
#include "mem.h"

#include "assert.h"

enum { NOT_FOUND = (uint)-1 };

typedef bool (*OrderFunc)(const void* a, const void* b);

#include "arithmetic.h"
#include "time.h"
#include "dlmalloc.h"
#include "vector.h"
#include "deque.h"
#include "trace.h"
#include "ovector.h"
#include "odeque.h"
#include "ring.h"
#include "set.h"
#include "platform.h"
#include "freelist.h"
#include "pstring.h"
#include "thread.h"
#include "error.h"
#include "netaddr.h"
#include "network.h"
#include "board.h"

#endif // __ENETCORE_H__
