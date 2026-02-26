// Copyright (c) 2026 Jan Brittenson
// See LICENSE for details.

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define _console _uart7

#ifdef DEBUG
#define _trace _uart7
//#define _trace _swo
#else
#define _trace _uart7
#endif

// Use SRAM1+SRAM2 as 256k IRAM
// Use SRAM4 as 64k Ethernet buffer

// A lot of #define's here unfortunately, resolve symbols later

#define THREAD_DATA_SIZE  ((sizeof(Thread) + 3) & ~3) // sizeof (Thread), aligned

// all flash sections
#define TEXT_REGION_START (BASE_FLASHB1)
#define TEXT_REGION_SIZE  ((uintptr_t)&_etext - TEXT_REGION_START)

// Main (default) internal RAM region, contains thread stacks.
#define IRAM_REGION_SIZE (MAIN_THREAD_STACK + INTR_THREAD_STACK + 2*THREAD_DATA_SIZE + 8)
#define IRAM_REGION_START ((uintptr_t)&__stack_top - IRAM_REGION_SIZE)

#define MALLOC_REGION_START ((uintptr_t)&_bss_end)
#define MALLOC_REGION_SIZE  (IRAM_REGION_START - MALLOC_REGION_START)

// Set to 0 to disable nested interrupts for debugging
#if 0

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
    IPL_ADC      = IPL_MIN  - 3, // ADC
    IPL_HASH     = IPL_MIN  - 3, // HASH
    IPL_DAC      = IPL_CRIT + 1, // DAC
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
    IPL_ADC      = IPL_SOFT,  // ADC
    IPL_HASH     = IPL_SOFT,  // HASH
    IPL_DAC      = IPL_SOFT,  // DAC
};
#endif

// DMA stream and channel usage
enum {
    // DMA1
    DMA_STREAM_UART7_TX = 0xffffff00, // Always 0
#define DMA_TARGET_UART7_TX Stm32Dma::Target::uart7_tx_dma

    DMA_STREAM_UART5_TX = 0xffffff00, // Always 0
#define DMA_TARGET_UART5_TX Stm32Dma::Target::uart5_tx_dma

#define DMA_PRIORITY_UART7  Stm32Dma::Priority::MEDIUM
#define DMA_PRIORITY_UART5  Stm32Dma::Priority::MEDIUM
};

enum {
    UART7_SENDQ_SIZE = 1024,
    UART7_RECVQ_SIZE = 64,
    UART5_SENDQ_SIZE = 1024,
    UART5_RECVQ_SIZE = 1024
};

// External SRAM in bank 1
#define XSRAM_BANK Fmc::Bank::BANK1

 // Base address for bank 1, sub bank 1 (NE1)
#define XSRAM_BASE BASE_FMCB1


// FMC SRAM mode 1 timing
enum {
    FMC1_FREQ      = 60000000,                    // Run memory at 6MHz
    HCLK_PER_CYC1  = AHB_FREQ/FMC1_FREQ,          // HCLKs per bus cycle
    BUS_TURN_CLK1  = HCLK_PER_CYC1/2,             // Bus turn HCLKs
    DATA_CLK1      = (HCLK_PER_CYC1 - BUS_TURN_CLK1), // Data cycle HCLKs
    DATA_SETUP_CLK1= int(AHB_FREQ*4.5e-12)+1,     // Data setup (actually address setup for data access)
    DATA_HOLD_CLK1 = DATA_CLK1 - DATA_SETUP_CLK1  // Data hold, HCLKs
};


// Enetcore configuration parameters

#define VARIANT "Enetcore"

#define USE_LITERALS			// Optimize dup/free of literals

// #define GCC_TLS

// ConnectedSocket timeout
enum { SHUTDOWN_TIMEOUT = 10000 };


// dlmalloc config - see dlmalloc.h
#define USE_LOCKS 1
#define MLOCK_T Mutex
#define INITIAL_LOCK(l) ((void)0)
#define ACQUIRE_LOCK(l) ((l)->Lock(), 0)
#define RELEASE_LOCK(l) ((l)->Unlock(), 0)
#define LOCK_INITIALIZER

#define HAVE_MORECORE 1
#define MORECORE(X)  _malloc_region.GetMem((X))
#define HAVE_MMAP 0
#define HAVE_MREMAP 0
#define malloc_getpagesize _malloc_region.GetPageSize()
#define DEFAULT_TRIM_THRESHOLD 16384 /* Trim is dirt cheap */
#define MORECORE_CANNOT_TRIM 1		 // But even cheaper is not to do it at all
#define INSECURE 0					 // Integrity checks
#define MSPACES 0					 // No malloc spaces
#define LACKS_ERRNO_H 1
#define LACKS_STRING_H 1
#define LACKS_UNISTD_H 1
#define LACKS_SYS_MMAN_H 1
#define LACKS_STDLIB_H 1
#define LACKS_FCNTL_H 1
#define LACKS_STDIO_H 1
#define LACKS_SYS_TYPES_H 1
#define LACKS_SBRK 1
#define MALLOC_FAILURE_ACTION

#ifdef DEBUG
#define MALLOC_DEBUG				 // Extra debug checking
#endif

// HashTable default reservation, eviction depth (for cuckoo)
// See hashtable.h for more info
enum { HASHTABLE_DEFAULT_RESERVE = 64 };
enum { HASHTABLE_EVICTION_DEPTH = 10 };

enum { MAIN_THREAD_STACK = 2048};
enum { INTR_THREAD_STACK = 2048 };

enum { THREAD_DEFAULT_STACK = 2048 }; // Default thread stack size
enum { THREAD_DEFAULT_PRIORITY = 50 }; // Default thread priority

// IOScheduler settings
enum { IO_FUDGE = 10 };	  // I/O scheduler timing granularity, in msec
enum { IO_STACK_SIZE = 2048 };	// Stack size for I/O threads

// String
#define STRING_FREELIST			// Enable freelisting of String objects

// Interrupt priorities
// Add or remove as needed, but PRIO_NORM is required even if it's the
// only priority.
enum {
	PRIO_INTR = 0,				//  Highest - interrupt thread
	PRIO_HIGH,
	PRIO_NORM,
	PRIO_LOW,
	NUM_PRIO					// Lowest
};


// Watchdog ping interval
enum { WATCHDOG_PING_INTERVAL = 5 };	// Time between pings, in seconds

// Various configuration checks
static_assert((MAIN_THREAD_STACK & 3) == 0);
static_assert((INTR_THREAD_STACK & 3) == 0);

#endif // _CONFIG_H_
