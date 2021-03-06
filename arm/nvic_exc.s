@@@ -*- mode: asm; asm-comment-char: "@"; -*-
@@@
@@@ Copyright (c) 2018 Jan Brittenson
@@@ See LICENSE for details.
	
.syntax    unified
.text
.force_thumb

.set PCBOFF, 0

@@@ Nesting NVIC exception handlers
@@@
@@@ On an exception, the following has been pushed onto the stack:
@@@    0x1c - PSR
@@@    0x18 - PC
@@@    0x14 - LR
@@@    0x10 - R12
@@@    0x0c - R3
@@@    0x08 - R2
@@@    0x04 - R1
@@@    SP+0 - R0
@@@ LR contains a special EXC_RETURN value (0xffffffX)
@@@ 
@@@ If the FPU is enabled it also stores all 16 FPU registers on the stack.
@@@ The value in LR indicates whether this is the case.  No exception handler
@@@ should touch the D registers without saving them first.
@@@
@@@ For IRQs (IDs >= 16) a global table, __token_table contains
@@@ parameters to pass to the handler.  For system exceptions (ID < 16),
@@@ NULL is provided.  The purpose of the token is for the handler to map
@@@ to an instance.

    .align
	
.func exc_handler
.global exc_handler

exc_handler:
    ldr    r1, =__handler_table
    mrs    r2, ipsr
    and    r2, r2, #0xff           @ Only ff rather than 1ff
    ldr    r1, [r1, +r2, lsl#2]    @ R1 = handler 
    cmp    r2, #16
    subge  r2, r2, #16
    ldrge  r0, =__token_table
    ldrge  r0, [r0, +r2, lsl#2]    @ R0 = _token_table[ (ipsr & 0xff) - 16 ] 
    movlt  r0, #0
    bx     r1                      @ Jump to handler 

.endfunc

@@@ PendSV: used to perform a context switch
@@@ 
@@@ We leave R0-R3, LR, PC, PSR on the thread PSP and only save
@@@ R5-R11,PSP

.func pendsv_handler
.global pendsv_handler

	.align
pendsv_handler:

@@@ Save thread state 

    mrs    r0, psp                 @ R0 = PSP
	stmfd  r0!, {r4-r11}           @ Save R11-R4 on thread stack
    ldr    r1, =_ZN6Thread10_curthreadE @ Thread::_curthread
    ldr    r1, [r1]
    str    r0, [r1, #PCBOFF]       @ Save thread PSP
    
@@@ Call handler 
	
    push   {lr}                    @ Save exception LR across handler call
	ldr    r1, =_ZN6Thread20ContextSwitchHandlerEPv+1 @ Thread::ContextSwitchHandler
    blx    r1                      @ Call handler 

@@@ Load up new thread context and return to it 

    ldr    r1, =_ZN6Thread10_curthreadE @ Thread::_curthread
    ldr    r1, [r1]
	ldr    r0, [r1, #PCBOFF]       @ R0 = saved PSP
	ldmfd  r0!, {r4-r11}           @ Restore R4-R11
    msr    psp, r0                 @ Restore thread PSP
    pop    {pc}                    @ PC = EXC_RETURN 

.endfunc
.end
