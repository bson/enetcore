# enetcore

Emdedded networking core for ARM Cortex-M4 microcontrollers; NXP LPC4000 in particular.  This was refreshed from an old ARM7TDMI codebase... you may still find vestigial remnants of this.

## about

enetcore is not a real-time operating system; instead it's a set of patterns used to build real-time applications to run
on bare metal.  Despite this, it does provide a threading abstraction and a deadline scheduler with O(N) time - e.g.
signal a condition variable or event object and it determines what to do, if anything, in exactly one pass through
the runnable list.

The memory model is partitioned, broken into "regions".  A region is mapped by linker to e.g. internal NOR flash, internal
SRAM, internal I/O SRAM, external SRAM, external DRAM, etc.  Internal SRAM is often single-cycle and enetcore prefers to
use it for system state, thread state, thread stacks (effectively making stack temporaries compare to register temporaries),
and application global state that benefits from fast access.  Ethernet, DMA, USB, and other I/O buffers are placed in the
SRAM window they can access.  Each region implement sbrk-style allocation without release (basically fill it up like a glass);
this permits dlmalloc to sit on top of regions to form a dynamic heap.  It's recommended that this be used for bulk memory
usage, and placed in slower (external) memory.  In addition, regions are used for .bss, .data, etc and a means to do control
memory layout at link time.

Supports basic DHCP and UDP.  I will add a TCP implementation that closely mimics Linux (e.g. bicubic recovery, fast start).

The purpose of this system was to facilitiate threaded, modern, C++ style development (but without bloat like exceptions and
reflection and without generics other than for actual specialization), but without 
needing the resources to run an entire Linux system. 

## Targeting Cortex-M4

(Note: M0 isn't that different.)

The memory is split into three regions:

* malloc - memory available for malloc
* iram   - internal RAM; contains thread context, stacks, and other large permanent objects.
* pram   - two banks of 16kB peripheral RAM, on separate buses.  Used for Ethernet and USB I/O buffers.

On startup the MSP (main stack ptr) is placed at the top of IRAM.  Later during initialization this becomes the PSP (process stack ptr) for the main thread.  The MSP is moved down in memory and used purely as an interrupt stack, for nested interrupts.
Both of these sit within the IRAM region, but are blocked off as unavailable.  (Regions support a "reserve", which is what's used here.  That amount at the end of a region becomes unavailable.)

There are two forms of exception handlers:

* A basic, fast variety that is just a C or assembly function.  It currently can't force a context switch. (I will add this
 later though by permitting it to post a pending software interrupt, SVCall, at a lower interrupt priority, which will 
 then enact a context switch.)  These operate at high interrupt priorities and are intended for tasks that need to be done NOW.
* A standard variety, at lower priorities.  These enter a common exception handler, which saves execution state for the
current thread and call the registered handler for that exception/IRQ.  As a parameter it passes a void* cookie, which the
handler can interpret as an object or data pointer, or something else.  This token has been registered for that handler.  For
nested interrupts, it only saves context at the outermost level.

Priority based interrupt nesting is fully supported, and each handler when registered also provides a priority.

When idle it waits for interrupt (WFI).

### LPC4000 peripherals currently supported:

TIMER, UART, SSP, I2C, PWM, ENET, USB device, GPIO, SYSTICK , NVIC, EEPROM, CRC, EINTR, PLL
(Not all features, but addititional features aren't hard to add.)

### Other support

SD cards, FAT16/32 file systems, UDP, DHCP, DNS, SSD1963 displays, HD44780, SSD1303, SSD1306, AD5667R DAC, ADS115

### To do:

ADC, DAC, GPDMA, I2S, improved I2C, Sleep modes, TCP

### Board support

Sky Blue, to be open sourced with this software.
