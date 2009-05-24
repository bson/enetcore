#ifndef __BOARD_H__
#define __BOARD_H__

// This file maps hardware to canonical names and provides external decls.

extern Vic _vic;




#include "cs8900a.h"
typedef MacCS8900a Ethernet;

// These are defined in hardware.cpp
extern Ethernet _eth0;

#endif // __BOARD_H__
