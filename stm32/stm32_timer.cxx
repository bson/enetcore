// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "lpc_timer.h"
#include "thread.h"

template<typename Counter>
void Stm32Timer::Interrupt(void* token) {
    ((Stm32Timer*)token)->HandleInterrupt();
}

template Stm32Timer<uint32_t>::Interrupt(void*);
template Stm32Timer<uint16_t>::Interrupt(void*);
