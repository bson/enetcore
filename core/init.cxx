// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"

[[__weak]] extern void (*__preinit_array_start []) (void);
[[__weak]] extern void (*__preinit_array_end []) (void);
[[__weak]] extern void (*__init_array_start []) (void);
[[__weak]] extern void (*__init_array_end []) (void);

void init_array() {
    size_t count, i;
    
    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start[i]();
    
    //_init();
    
    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++)
        __init_array_start[i]();
}
