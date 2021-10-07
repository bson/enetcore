// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"

// THUMB is 1 to force Thumb
#ifdef CORTEX_M
#define THUMB 1
#else
#define THUMB 0
#endif

extern uintptr_t __vector_table[]; // Allocated by linker

uintptr_t NVic::_handler_table[];
void* NVic::_token_table[];

extern "C" {
extern void exc_handler();
extern void pendsv_handler();
}

void NVic::Init(IRQHandler handler, uint8_t prio) {
    assert(prio && prio < IPL_NUM);

    prio *= IPL_QUANTUM;

    ScopedNoInt G;

    // Default priority pattern
    const uint32_t ppat = (prio << 24) | (prio << 16) | (prio << 8) | prio;

    // The first two entries in the table are SP, Reset.  Fill those
    // in for good measure.
    __vector_table[0] = (uintptr_t)&__stack_top;
    __vector_table[1] = 0x400;
    
    // Everything else gets a default handler
    for (uintptr_t* p = __vector_table + 2; p < __vector_table + INT_NUM + 16; )
        *p++ = (uintptr_t)handler | THUMB;
    
    memset(_token_table, 0, sizeof _token_table);
    
#if 0
    SHPR1 = SHPR2 = SHPR3 = ppat;
#endif
    volatile uint32_t *icpr = &IPR0;

    for (uint i = 0; i < 11; ++i)
        *icpr++ = ppat;

    VTOR = (uint32_t)__vector_table;
}

void NVic::InstallIRQHandler(uint irq, IRQHandler handler, uint8_t prio, void* token,
                             bool fast) {
	assert(irq < INT_NUM);
    assert(prio && prio < IPL_NUM);

    ScopedNoInt G;

    if (fast) {
        __vector_table[16 + irq] = (uintptr_t)handler | THUMB;
    } else {
        __vector_table[16 + irq] = (uintptr_t)exc_handler | THUMB;
        _handler_table[16 + irq] = (uintptr_t)handler | THUMB;
        _token_table[irq] = token;
    }

    SetIRQPriority(irq, prio);
    DisableIRQ(irq);
    ClearPendingIRQ(irq);
}

void NVic::InstallSystemHandler(uint id, IRQHandler handler, uint8_t prio, bool fast) {
	assert(id < 16);
    assert(prio && prio < IPL_NUM);

    prio *= IPL_QUANTUM;

    ScopedNoInt G;
        
    if (fast) {
        __vector_table[id] = (uintptr_t)handler | THUMB;
    } else {
        __vector_table[id] = (uintptr_t)exc_handler | THUMB;
        _handler_table[id] = (uintptr_t)handler | THUMB;
    }        

    // Only some system interrupts have configurable priority: 4, 5, 6, 11, 14, 15
    const uint32_t configurable = 0b1100100001110000;

    if (configurable & (1 << id)) {
        id -= 4;
        volatile uint32_t *shpr = &SHPR1;
        const uint32_t mask = 0xff << ((id & 3) * 8);
        shpr[id/4] = (shpr[id/4] & ~mask) | (prio << ((id & 3) * 8));
    }
}


void NVic::InstallCSWHandler(uint id, uint8_t prio) {
	assert(id < 16);
    assert(prio && prio < IPL_NUM);

    prio *= IPL_QUANTUM;

    ScopedNoInt G;
        
    __vector_table[id] = (uintptr_t)pendsv_handler | THUMB;

    // Only some system interrupts have configurable priority: 4, 5, 6, 11, 14, 15
    const uint32_t configurable = 0b1100100001110000;

    if (configurable & (1 << id)) {
        id -= 4;
        volatile uint32_t *shpr = &SHPR1;
        const uint32_t mask = 0xff << ((id & 3) * 8);
        shpr[id/4] = (shpr[id/4] & ~mask) | (prio << ((id & 3) * 8));
    }
}


void NVic::SetIRQPriority(uint irq, uint8_t prio) {
	assert(irq < INT_NUM);
    assert(prio < IPL_NUM);

    prio *= IPL_QUANTUM;

    const uint32_t mask = 0xff << (8 * (irq & 3));
    const uint32_t val  = prio << (8 * (irq & 3));
    volatile uint32_t* pri = &IPR0 + irq/4;

    ScopedNoInt G;
    *pri = (*pri & ~mask) | val;
}

void NVic::EnableIRQ(uint irq)
{
	assert(irq < INT_NUM);

    volatile uint32_t* iser = &ISER0 + irq/32;
    *iser = 1 << (irq & 31);
}

void NVic::DisableIRQ(uint irq)
{
	assert(irq < INT_NUM);

    volatile uint32_t* icer = &ICER0 + irq/32;
    *icer = 1 << (irq & 31);
}

bool NVic::PendingIRQ(uint irq)
{
	assert(irq < INT_NUM);

    const volatile uint32_t* ispr = &ISPR0 + irq/32;
    return *ispr & (1 << (irq & 31));
}

void NVic::ClearPendingIRQ(uint irq)
{
	assert(irq < INT_NUM);

    volatile uint32_t* icpr = &ICPR0 + irq/32;
    *icpr = 1 << (irq & 31);
}
