// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef _CORTEX_M4_H_
#define _CORTEX_M4_H_

#include "defs.h"
#include "bits.h"

// Various constants
enum {
    CPUTAPID = 0x410fc241       // Cortex-M4 CPUID
};

enum {
    NVIC_BASE     = 0xfffff000,   // CM4
};

extern "C" {

// ARM NVIC registers
DEV_CTL_REG(ISER0);
DEV_CTL_REG(ISER1);
DEV_CTL_REG(ICER0);
DEV_CTL_REG(CER1);
DEV_CTL_REG(ISPR0);
DEV_CTL_REG(ISPR1);
DEV_CTL_REG(ICPR0);
DEV_CTL_REG(ICPR1);
DEV_CTL_REG(IABR0);
DEV_CTL_REG(IABR1);
DEV_CTL_REG(IPR0);
DEV_CTL_REG(IPR1);
DEV_CTL_REG(IPR2);
DEV_CTL_REG(IPR3);
DEV_CTL_REG(IPR4);
DEV_CTL_REG(IPR5);
DEV_CTL_REG(IPR6);
DEV_CTL_REG(IPR7);
DEV_CTL_REG(IPR8);
DEV_CTL_REG(IPR9);
DEV_CTL_REG(IPR10);
DEV_CTL_REG(STIR);

// ARM MPU
DEV_CTL_REG(MPU_TYPE);
DEV_CTL_REG(MPU_CTRL);
DEV_CTL_REG(MPU_RNR);
DEV_CTL_REG(MPU_RBAR);
DEV_CTL_REG(MPU_RASR);
DEV_CTL_REG(MPU_RBAR_A1);
DEV_CTL_REG(MPU_RASR_A1);
DEV_CTL_REG(MPU_RBAR_A2);
DEV_CTL_REG(MPU_RASR_A2);
DEV_CTL_REG(MPU_RBAR_A3);
DEV_CTL_REG(MPU_RASR_A3);

// ARM SYSTICK
DEV_CTL_REG(SYST_CSR);
DEV_CTL_REG(SYST_RVR);
DEV_CTL_REG(SYST_CVR);
DEV_CTL_REG(SYST_CALIB);

// ARM Core Cortex-M4 system control
DEV_CTL_REG(CPUID);
DEV_CTL_REG(ACTLR);
DEV_CTL_REG(ICSR);
DEV_CTL_REG(VTOR);
DEV_CTL_REG(AIRCR);
DEV_CTL_REG(SCR);
DEV_CTL_REG(CCR);
DEV_CTL_REG(SHPR1);
DEV_CTL_REG(SHPR2);
DEV_CTL_REG(SHPR3);
DEV_CTL_REG(SHCRS);
DEV_CTL_REG(CFSR);
DEV_CTL_REG_8(MMFSR);
DEV_CTL_REG_8(BFSR);
DEV_CTL_REG_16(UFSR);
DEV_CTL_REG(HFSR);
DEV_CTL_REG(MMAR);
DEV_CTL_REG(BFAR);
DEV_CTL_REG(AFSR);

};

#ifdef __GNUC__
static inline void WaitForInterrupt() {
#ifdef ENABLE_WFI
    asm volatile("wfi" : : : );
#endif
}

static inline uint32_t DisableInterrupts() {
	uint32_t prev;
	asm volatile("mrs %0, primask; movs r1, #1; msr primask, r1"
                 : "=r"(prev)
                 :  : "r1" );
	return prev & 1;
}

static inline void RestoreInterrupts(uint prev) {
#if CORTEX_M == 0
    asm volatile("msr primask, %0; isb" : : "r"(prev & 1) : );
#else
    asm volatile("msr primask, %0; isb; isb" : : "r"(prev & 1) : );
#endif    
}

[[__finline]] static inline void EnableInterrupts() {
    RestoreInterrupts(0);
}

// Set current IPL, returning previous
static inline uint32_t SetIPL(uint32_t ipl) {
    uint32_t result;
    asm volatile("mrs %0, basepri; msr basepri, %1"
                 : "=r"(result)
                 : "r"((ipl % 32) * 8)
                 : );
    return result;
}

// Return current IPL
static inline uint32_t GetIPL() {
    uint32_t result;
    asm volatile("mrs %0, basepri"
                 : "=r"(result) : :);
    return result/8;
}

// Test if interrupts are enabled
static inline bool IntEnabled() {
	uint32_t primask;
	asm volatile("mrs %0, primask" : "=r"(primask) : : );
	return (primask & 1) == 0;
}

// True if in an exception handler
static inline bool InExceptionHandler() {
    uint32_t ipsr;
    asm volatile("mrs %0, ipsr" : "=r"(ipsr) : : );
    return (ipsr & 0xff) != 0;
}

// Post PendSV - this vectors to Thread::ContextSwitch() and can be used
// by an anwrapped fast interrupt handler to chain to a context switch
// (at a lower IPL).
static inline void PostContextSwitch() {
    ICSR |= BIT28;              // Set PendSV as pending
}
    
// Save FP state
static inline void SaveFP(uint32_t* to) {
    asm volatile("vstmia %0!,{s0-s31}" : : "r"(to) : "mem");
}

// Load FP state
static inline void LoadFP(uint32_t* from) {
    asm volatile("vldmia %0!,{s0-s31}" : : "r"(from) : "mem");
}

#else
#error "Unsupported compiler"
#endif

// RAII reentrant version of interrupt masking
class ScopedNoInt {
    uint32_t _save;
public:
    ScopedNoInt() {
        _save = DisableInterrupts();
    }
    ~ScopedNoInt() {
        RestoreInterrupts(_save);
    }
    void Reacquire() {
        DisableInterrupts();
    }
    void Lock() {
        DisableInterrupts();
    }
    void Unlock() {
        RestoreInterrupts(_save);
    }
};

//// Thread support

// Structure included by Thread to use to store context
//
// Note: R0-R3, R12, LR, PC, PSR are kept on the thread stack where
// exceptions deposit them.  R4-R11 are pushed on context switch
// immediately following the exception state.  Only the PSP is kept in
// thread data.

struct PcbPrimitive {
    uint32_t psp;           // Thread PSP
};


// FP state
struct FPState {
    uint32_t s[32];
}

#if defined (__GNUC__)

// The following stuff is all #define's because they need to be
// inlined _verbatim_ in a specific environment.  A potential function
// call or other overhead in around of these blocks of code could
// cause serious damage.

// Return current SP, for various debug purposes
#define GetSP(SP)															\
	asm volatile ("mov %0, sp" : "=r"(SP) : : )

// Set up stacks.
// The current MSP becomes a thread PSP.
// The MSP is set to the start of the interrupt stack.
#define StackStrap(INTR_TOS) \
    asm volatile( \
    "mov   r0, sp;" \
    "msr   psp, r0;"  /* PSP = MSP */ \
    "mrs   r0, control;" \
    "orr   r0, r0, #2;" \
    "msr   control, r0;" /* CONTROL |= SPSEL, switch to PSP */ \
    "msr   msp, %0;" /* Set up MSP */ \
    : : "r"(INTR_TOS) : "r0" )

// For debugging assistance...
struct EX {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};
[[gnu::weak]] volatile EX ex_dummy;           // So the type info isn't dropped

#else
#error "implement this"
#endif

#endif // _CORTEX_M4_H_
