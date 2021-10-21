// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "params.h"
#include "stm32f4x.h"

// Set to 0 to disable nested interrupts for debugging

#if 1

// Interrupt priority plan.  Values are 8 bits with the low 3 bits unused.
// Note: lower values are higher priority.
enum {
    IPL_QUANTUM  = 8,

    // Priority bands
    IPL_MAX      = 1,           //  1 Max priority
    IPL_EXC      = 4,           //  3 Exceptions
    IPL_CRIT     = 12,          //  8 Real-time critical functions
    IPL_SOFT     = 17,          //  5 Soft real-time functions
    IPL_COMM     = 27,          // 10 Various other communications
    IPL_MIN      = 31,          //  5 Low priority
    IPL_NUM      = 32,

    // Scheduler IPL.  This is the highest IPL that can make calls other
    // than Wake calls.
    IPL_SCHED    = IPL_CRIT+1,

    // Hardware
    IPL_UNEXP    = IPL_MAX,     // Unexpected interrupt
    IPL_NMI      = IPL_MAX,
    IPL_HW_EXC   = IPL_MAX,     // Hardware exceptions: bus error, etc
    IPL_SW_EXC   = IPL_MIN - 2, // Software exceptions: SV call
    IPL_CSW      = IPL_MIN - 1, // Software context switch (PendSV)
    IPL_SYSTICK  = IPL_EXC,     // SysTick - high priority
    IPL_CLOCK    = IPL_MIN - 1, // Real-time clock
    IPL_SYSTIMER = IPL_CRIT - 1, // Scheduler
    IPL_UART     = IPL_COMM - 5, // UART
    IPL_I2C      = IPL_COMM - 2, // I2C
    IPL_SPI      = IPL_COMM - 2, // SPI
    IPL_USB      = IPL_COMM - 7, // USB
    IPL_GPIO     = IPL_MIN - 5,  // Pin interrupts
    IPL_ENET     = IPL_COMM - 4, // Ethernet controller
    IPL_MISR     = IPL_MIN  - 5, // PHY MISR interrupt (link change, etc)
    IPL_DMA      = IPL_MIN  - 3, // DMA (generic)
};
#else
// Non-nested interrupt priorities.  Except HW faults.
enum {
    IPL_SOFT     = 17,

    IPL_MIN      = 31,
    IPL_MAX      = 1,
    IPL_NUM      = 32,

    // Values to use
    IPL_UNEXP    = IPL_SOFT,  // Unexpected interrupt
    IPL_NMI      = IPL_SOFT,
    IPL_HW_EXC   = IPL_SOFT-5, // Hardware exceptions: bus error, etc
    IPL_SW_EXC   = IPL_SOFT,  // Software exceptions: SV call
    IPL_CSW      = IPL_SOFT+1, // Software context switch (PendSV)
    IPL_SYSTICK  = IPL_SOFT,  // SysTick
    IPL_CLOCK    = IPL_SOFT,  // Real-time clock
    IPL_SYSTIMER = IPL_SOFT,  // Scheduler
    IPL_UART     = IPL_SOFT,  // UART
    IPL_I2C      = IPL_SOFT,  // I2C
    IPL_SPI      = IPL_SOFT,  // SPI
    IPL_USB      = IPL_SOFT,  // USB
    IPL_GPIO     = IPL_SOFT,  // Pin interrupts
    IPL_ENET     = IPL_SOFT,  // Ethernet controller
    IPL_MISR     = IPL_SOFT,  // PHY MISR interrupt (link change, etc)
    IPL_DMA      = IPL_SOFT,  // DMA
};
#endif

extern "C" {
// These symbols are generated by the linker in link.cmd
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss_start;
extern uint8_t _bss_end;
extern uint8_t _etext;
extern uint8_t _bss_start;
extern uint8_t _bss_end;
extern uint8_t __stack_top;
};

void Unexpected_Interrupt(void*);
void Hard_Fault_Exception(void*);
void NMI_Handler(void*);
void Mem_Man_Fault_Exception(void*);
void Bus_Fault_Exception(void*);
void Usage_Fault_Exception(void*);
void SVCall_Handler(void*);

void fault0(uint num);

[[__finline]] static inline void fault(uint num, bool captive = true) {
    while (captive) {
        fault0(num);
    }
}

// STM32F405 memory layout
//
//    0x0800 0000 Start Flash   1MB
//     end image                         _etext
//    0x080F FFFF End Flash
//
//    0x1000 0000  CCM RAM 64k  - unused for now
//    0x1000 FFFF
//
//    0x2000 0000  SRAM 112k  MALLOC_START             _bss_end
//    0x2001 BFFF
//    0x2001 C000  SRAM 16k   IRAM_START
//    0x2001 FFFF             IRAM_START + IRAM_SIZE   __stack_top
//
//    The last two adjacent SRAM sections are combined into a 128k region
//
// The existing main stack (MSP) is inside IRAM, and set aside
// setting a reserve equal to MAIN_THREAD_STACK plus INTR_THREAD_STACK.
// Thread data structures are allocated using the IRAM region as well.
// 

// A lot of #define's here unfortunately, resolve symbols later

#define THREAD_DATA_SIZE  ((sizeof(Thread) + 3) & ~3) // sizeof (Thread), aligned

// all flash sections
#define TEXT_REGION_START (0x08000000)
#define TEXT_REGION_SIZE  ((uintptr_t)&_etext - TEXT_REGION_START)

#ifdef ENABLE_PANEL
#enable ENABLE_TSC2046
#define UI_THREAD_TERM (UI_THREAD_STACK + THREAD_DATA_SIZE)
#else
#define UI_THREAD_TERM 0
#endif

// Internal RAM region, contains thread stacks.
#define IRAM_REGION_SIZE (MAIN_THREAD_STACK + INTR_THREAD_STACK + 2*THREAD_DATA_SIZE + 8 + UI_THREAD_TERM)
#define IRAM_REGION_START ((uintptr_t)&__stack_top - IRAM_REGION_SIZE)

#define MALLOC_REGION_START ((uintptr_t)&_bss_end)
#define MALLOC_REGION_SIZE  (IRAM_REGION_START - MALLOC_REGION_START)

#include "stm32f4x/timer.h"

#endif // __HARDWARE_H__
