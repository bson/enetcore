@@@ -*- mode: asm; asm-comment-char: "@"; -*-
@@@
@@@ Copyright (c) 2018-2021 Jan Brittenson
@@@ See LICENSE for details.

.syntax  unified
.section .startup
.force_thumb

.func   _startup
_startup:
.global _startup

# Exception Vectors

.weakref NMI_Handler, _unimpl_vect
.weakref HardFault_Handler, _unimpl_vect
.weakref MemManage_Handler, _unimpl_vect
.weakref BusFault_Handler, _unimpl_vect
.weakref Usage_Handler, _unimpl_vect
.weakref SysTick_Handler, _unimpl_vect
.weakref SVCall_Handler, _unimpl_vect

    . = 0
    
_vectors:_vectors:
    .word   __stack_top         /* Initial SP */
    .word   _reset+1            /* Reset vector */
    .word   NMI_Handler+1      /* NMI */
    .word   HardFault_Handler+1 /* Hard fault */
    .word   MemManage_Handler+1 /* Memory management fault */
    .word   BusFault_Handler+1 /* Bus fault */
    .word   Usage_Handler+1    /* Usage fault */
    .word   0
    /* Filled in with checksum by flash programmer */
    /* skipping reserved entries */
    .= 0x2c
    .word   SVCall_Handler+1   /* SVCall */
    .word   _unimpl_vect+1     /* Debug use */
    .word   _unimpl_vect+1     /* Reserved */
    .word   _unimpl_vect+1     /* PendSV */
    .word   SysTick_Handler+1  /* SysTick */

    /* IRQs */
    .rept   (0x400-0x40)/4
    .word   _unimpl_vect+1
    .endr

    . = 0x400
.thumb_func
_reset:
    /* turn off interrupts */
    mov      r0, #1
    msr      primask, r0

    /* delay loop */
    .ifdef DELAY
    ldr     r0, =DELAY
3:  subs    r0, r0, #1
    bne     3b
    .endif

    /* start main stack at top of internal RAM */
    ldr     r0, =__stack_top
    mov     sp, r0

    /* copy .data section (Copy from ROM to RAM) */
    ldr     r1, =_etext
    ldr     r2, =_data
    ldr     r3, =_edata
1:
    cmp     r2, r3
    ldrlo   r0, [r1], #4
    strlo   r0, [r2], #4
    blo     1b

    /* Clear .bss section (Zero init)  */
    mov     r0, #0
    ldr     r1, =_bss_start
    ldr     r2, =_bss_end
2:
    cmp     r1, r2
    strlo   r0, [r1], #4
    blo     2b

    /* Create dummy frame */
    mov     r7, r0
    mov     lr, r0
    push    {r7,lr}

    mov     r11, #0  /* FP=0 so GDB can see where the stack ends */
    bl      _Z7hwinit0v
    bl      _Z10init_arrayv
    bl      _Z6hwinitv
    bl      main
    b       .       /* Wait for watchdog reset, if enabled */

.align
.thumb_func

_unimpl_vect:
    .ifdef DEBUG
    bkpt #1
    .else
    b       .
    .endif

.endfunc

.end
