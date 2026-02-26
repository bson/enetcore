// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __ENETCORE_H__
#define __ENETCORE_H__

#include <new>

#define ENETCORE 1

#include "compiler.h"

enum { NOT_FOUND = (uint32_t)-1 };

#define USE_LITERALS

#include "params.h"
#include "config.h"
#include "init.h"
#include "core/gpio.h"
#include "core/mem.h"

#include "core/assert.h"

typedef bool (*OrderFunc)(const void* a, const void* b);

#include "core/arithmetic.h"
#include "core/time.h"
#include "core/dlmalloc.h"
#include "core/vector.h"
#include "core/deque.h"
#include "core/trace.h"
#include "core/ovector.h"
#include "core/odeque.h"
#include "core/ring.h"
#include "core/set.h"
#include "core/platform.h"
#include "core/freelist.h"
#include "core/pstring.h"
#include "core/thread.h"
#include "core/error.h"
#include "core/netaddr.h"
#include "core/network.h"
#include "board.h"

#endif // __ENETCORE_H__
