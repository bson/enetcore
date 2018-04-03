// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __ARM_H__
#define __ARM_H__

// Generic ARM code and definitions

#define BIG_ENDIAN 0
#define LITTLE_ENDIAN 1
#define BYTE_ORDER LITTLE_ENDIAN


#ifdef __GNUC__
#define __irq   gnu::interrupt("IRQ"),__noinstrument
#else
#error "Unsupported compiler"
#endif

#endif // __ARM_H__
