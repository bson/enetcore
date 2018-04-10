// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#include "lpc.h"

// Set to 0 to disable nested interrupts for debugging

#if 1

// Interrupt priority plan.  Values are 8 bits with the low 3 bits unused.
// Note: lower values are higher priority.  (We reverse this purely for
// calculation purposes.)
enum {
    IPL_QUANTUM  = 8,

    // Priority bands
    IPL_MAX      = 1,           //  1 Max priority
    IPL_EXC      = 4,           //  3 Exceptions
    IPL_CRIT     = 12,          //  8 Real-time critical functions
    IPL_SOFT     = 17,          //  5 Soft real-time functions
    IPL_COMM     = 27,          // 10 Various other communications
    IPL_MIN      = 31,          //  5 Low priority
    IPL_NUM       = 32,

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
    IPL_ENET     = IPL_COMM - 4, // Ethernet controller
    IPL_MISR     = IPL_MIN  - 5, // PHY MISR interrupt (link change, etc)
    IPL_EEPROM   = IPL_COMM - 1, // EEPROM read/write
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
    IPL_ENET     = IPL_SOFT,  // Ethernet controller
    IPL_MISR     = IPL_SOFT,  // PHY MISR interrupt (link change, etc)
    IPL_EEPROM   = IPL_SOFT,  // EEPROM read/write
};
#endif

// Software priorities
enum {
    // Network service code, one lower than than ENET so ethernet interrupts
    // can preempt it.
    IPL_NETWORK  = IPL_ENET+1,
};

extern "C" {
// These symbols are generated by the linker in link.cmd
extern uint8_t _data;
extern uint8_t _edata;
extern uint8_t _bss_start;
extern uint8_t _bss_end;
extern uint8_t _etext;
extern uint8_t _iram;
extern uint8_t _eiram;
extern uint8_t _sram0;
extern uint8_t _esram0;
extern uint8_t _sram1;
extern uint8_t _esram1;
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

// Use TIMER3 for system clock
enum {
    CLOCK_BASE = TIMER3_BASE,
    CLOCK_IRQ  = TIMER3_IRQ,
    CLOCK_PCON = PCTIM3
};

// Memory layout:
//    0x00000000 FLASH sections
//
//    0x10000000 DATA   _data       - _edata
//               BSS    _bss_start  - _bss_end
//               MALLOC _bss_end    - _bss_end + MALLOC_SIZE
//               IRAM   MALLOC_END  - __stack_top
//    0x20000000 PRAM0   (16k)
//    0x20004000 PRAM1   (16k)
//
// The existing main stack (MSP) is inside IRAM, and set aside
// setting a reserve equal to MAIN_THREAD_STACK.
// 

// A lot of #define's here unfortunately, resolve symbols later

#define THREAD_DATA_SIZE  ((sizeof(Thread) + 3) & ~3) // sizeof (Thread), aligned

// all flash sections
#define TEXT_REGION_START (0x00000000)
#define TEXT_REGION_SIZE  ((uintptr_t)&_etext - TEXT_REGION_START)

#ifdef ENABLE_ENET
#define NET_THREAD_TERM (NET_THREAD_STACK + THREAD_DATA_SIZE)
#else
#define NET_THREAD_TERM 0
#endif
#ifdef ENABLE_USB
#define USB_THREAD_TERM (USB_THREAD_STACK + THREAD_DATA_SIZE)
#else
#define USB_THREAD_TERM 0
#endif

// Internal RAM
#define IRAM_REGION_SIZE \
    (MAIN_THREAD_STACK + THREAD_DATA_SIZE + INTR_THREAD_STACK \
     + NET_THREAD_TERM + USB_THREAD_TERM)

#define IRAM_REGION_START ((uint8_t*)&__stack_top - IRAM_REGION_SIZE)

// Malloc region; sits between BSS and IRAM and is the remainder of internal SRAM
// XXX it would be simpler to simply make this nested inside IRAM and allocate it
// on startup.
#define MALLOC_REGION_START ((uint8_t*)(((uintptr_t)&_bss_end + 3) & ~3))
#define MALLOC_REGION_SIZE  (IRAM_REGION_START - MALLOC_REGION_START)

// We use the two peripheral 16k SRAM banks as a single uniform region
#define PRAM_REGION_START (&_sram0)
#define PRAM_REGION_SIZE  (&_esram1 - &_sram0)

#endif // __HARDWARE_H__
