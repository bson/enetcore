// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _DEFS_H_
#define _DEFS_H_

#define DEV_CTL_REG_8(NAME)   extern volatile uint8_t NAME
#define DEV_CTL_REG_16(NAME)  extern volatile uint16_t NAME
#define DEV_CTL_REG(NAME)     extern volatile uint32_t NAME

#endif // _DEFS_H_
