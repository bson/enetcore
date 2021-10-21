// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __ENETCORE_H__
#define __ENETCORE_H__

#include <new>

#define ENETCORE 1

#include "compiler.h"

enum { NOT_FOUND = (uint)-1 };

#define USE_LITERALS

#include "init.h"
#include "gpio.h"
#include "config.h"
#include "mem.h"

#include "assert.h"

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
