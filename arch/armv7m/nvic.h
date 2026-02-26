// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __NVIC_H__
#define __NVIC_H__

#include <stdint.h>


typedef void (*IRQHandler)(void *);

// System exceptions
enum {
    NMI_VEC = 2,
    HARD_FAULT_VEC = 3,
    MEM_MAN_FAULT_VEC = 4,
    BUS_FAULT_VEC = 5,
    USAGE_FAULT_VEC = 6,
    SVCALL_VEC = 11,
    PENDSV_VEC = 14,
    SYSTICK_VEC = 15
};

// Nested vectored interrupt controller.  This is a core ARM peripheral.
class NVic {
    // Handlers
    static uintptr_t _handler_table[INT_NUM+16] asm("__handler_table");

    // IRQ handler tokens.
    static void* _token_table[INT_NUM]  asm("__token_table");

public:
    NVic() { }

    // Initialize vector table
    static void Init(IRQHandler handler, uint8_t prio);

    // Install generic IRQ handler
    static void InstallIRQHandler(uint irq, IRQHandler handler, uint8_t prio, void* token,
                                  bool fast = false);

    // Install handler, enable
    [[__finline]]
    static void RouteIRQ(uint irq, IRQHandler handler, uint8_t prio, void* token,
                         bool fast = false) {
        InstallIRQHandler(irq, handler, prio, token, fast);
        EnableIRQ(irq);
    }

    // Install system exception handler
    static void InstallSystemHandler(uint id, IRQHandler handler, uint8_t prio,
                                     bool fast = false);

    // Install context switch handler
    static void InstallCSWHandler(uint id, uint8_t prio);

    static void SetIRQPriority(uint irq, uint8_t prio);

	static void EnableIRQ(uint irq);
	static void DisableIRQ(uint irq);

	// True if IRQ has a pending interrupt
	static bool PendingIRQ(uint irq);

	// Clear pending interrupt status - call prior to return
	static void ClearPendingIRQ(uint irq);

private:
    NVic(const NVic&) = delete;
    NVic& operator=(const NVic&) = delete;
};

#endif // __NVIC_H__
