// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef _CONFIG_H_
#define _CONFIG_H_

#define _console _usart3

#ifdef DEBUG
#define _trace _usart3
//#define _trace _swo
#else
#define _trace _usart3
#endif

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
#define UI_THREAD_TERM (UI_THREAD_STACK + THREAD_DATA_SIZE)
#else
#define UI_THREAD_TERM 0
#endif

// Internal RAM region, contains thread stacks.
#define IRAM_REGION_SIZE (MAIN_THREAD_STACK + INTR_THREAD_STACK + 2*THREAD_DATA_SIZE + 8 + UI_THREAD_TERM)
#define IRAM_REGION_START ((uintptr_t)&__stack_top - IRAM_REGION_SIZE)

#define MALLOC_REGION_START ((uintptr_t)&_bss_end)
#define MALLOC_REGION_SIZE  (IRAM_REGION_START - MALLOC_REGION_START)

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
    IPL_ADC      = IPL_MIN  - 3, // ADC
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
    IPL_DAC      = IPL_SOFT,  // DAC
};
#endif

// DMA stream and channel usage
enum {
    // DMA1
    DMA_STREAM_USART3_TX  = 3,
    DMA_CHANNEL_USART3_TX = 4,

    DMA_STREAM_DAC = 5,
    DMA_CHANNEL_DAC = 7,

    DMA_STREAM_SPI2_TX = 4,
    DMA_CHANNEL_SPI2_TX = 0,

    DMA_STREAM_SPI2_RX = 3,
    DMA_CHANNEL_SPI2_RX = 0,

#define DMA_PRIORITY_USART3  Stm32Dma::Priority::MEDIUM
#define DMA_PRIORITY_DAC Stm32Dma::Priority::HIGH
#define DMA_PRIORITY_SPI2 Stm32Dma::Priority::HIGH
};

enum {
    ADC1_PRESCALE = 8,
    USART3_SENDQ_SIZE = 128,
    USART3_RECVQ_SIZE = 8,
    UART4_SENDQ_SIZE = 1024,
    UART4_RECVQ_SIZE = 1024
};

// Panel is SRAM in bank 1
#define PANEL_BANK Fsmc::Bank::BANK1
//#define PANEL_BANK Fsmc::Bank::BANK2

enum {
    FSMC_PANEL_BASE = 0x60000000, // Base address for bank 1
//    FSMC_PANEL_BASE = 0x70000000, // Base address for bank 2
    FSMC_PANEL_DATA = FSMC_PANEL_BASE + (1 << 17), // Data access (A16)
    FSMC_PANEL_CMD  = FSMC_PANEL_BASE              // Cmd access
};


// FSMC SRAM mode 1 timing
enum {
    PANEL_BUS_FREQ       = 10000000,  // 10MHz bus freq (5MHz write/read speed)      
    PANEL_HCLK_PER_CYC   = HCLK/PANEL_BUS_FREQ,                 // HCLKs per bus cycle
    PANEL_BUS_TURN_CLK   = PANEL_HCLK_PER_CYC/2,                // Bus turn HCLKs
    PANEL_DATA_CLK       = (PANEL_HCLK_PER_CYC - PANEL_BUS_TURN_CLK), // Data cycle HCLKs
    PANEL_DATA_SETUP_CLK = int(HCLK*4.5e-12)+1, // Data setup (actually address setup for data access)
    PANEL_DATA_HOLD_CLK = PANEL_DATA_CLK - PANEL_DATA_SETUP_CLK // Data hold, HCLKs
};

// Timing specs from SSD1963 datasheet
static_assert((float)PANEL_BUS_TURN_CLK / (float)HCLK >= 9e-12);   // >= 9ns bus turn
static_assert((float)PANEL_DATA_CLK / (float)HCLK >= 9e-12);       // >= 9ns data cycle
static_assert((float)PANEL_DATA_HOLD_CLK / (float)HCLK >= 1e-12);  // >= 1ns data hold
static_assert((float)PANEL_DATA_SETUP_CLK / (float)HCLK >= 4e-12);  // >= 4ns data setup

static_assert(PANEL_DATA_HOLD_CLK >= 1);
static_assert(PANEL_DATA_HOLD_CLK <= 255);
static_assert(PANEL_DATA_SETUP_CLK <= 15);

enum {
    I2C_BUS_SPEED = 100000,
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

enum { UI_THREAD_STACK = 2048 };
enum { UI_THREAD_PRIORITY = 205 };

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
static_assert((UI_THREAD_STACK & 3) == 0);

#endif // _CONFIG_H_
