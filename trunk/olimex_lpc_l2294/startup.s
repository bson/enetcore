
/* Standard definitions of Mode bits and Interrupt (I & F) flags in PSRs (program status registers) */
.set  MODE_USR, 0x10            		/* Normal User Mode 										*/
.set  MODE_FIQ, 0x11            		/* FIQ Processing Fast Interrupts Mode 						*/
.set  MODE_IRQ, 0x12            		/* IRQ Processing Standard Interrupts Mode 					*/
.set  MODE_SVC, 0x13            		/* Supervisor Processing Software Interrupts Mode 			*/
.set  MODE_ABT, 0x17            		/* Abort Processing memory Faults Mode 						*/
.set  MODE_UND, 0x1B            		/* Undefined Processing Undefined Instructions Mode 		*/
.set  MODE_SYS, 0x1F            		/* System Running Priviledged Operating System Tasks  Mode	*/

.set  I_BIT, 0x80               		/* when I bit is set, IRQ is disabled (program status registers) */
.set  F_BIT, 0x40               		/* when F bit is set, FIQ is disabled (program status registers) */

.text
.arm

.global _startup
.func   _startup

_startup:

# Exception Vectors

_vectors:       b		_init
                ldr     pc, Undef_Addr
                ldr     pc, SWI_Addr
                ldr     pc, PAbt_Addr
                ldr     pc, DAbt_Addr
                nop							/* Reserved Vector (holds Philips ISP checksum) */
                ldr     pc, [pc,#-0xff0]	/* see page 71 of "Insiders Guide to the Philips ARM7-Based Microcontrollers" by Trevor Martin  */
                ldr     pc, FIQ_Addr

Undef_Addr:     .word   Undef_Exception
SWI_Addr:       .word   SWI_Trap
PAbt_Addr:      .word   Program_Abort_Exception
DAbt_Addr:      .word   Data_Abort_Exception
IRQ_Addr:       .word   Unexpected_Interrupt
FIQ_Addr:       .word   Unexpected_Interrupt
                .word   0
				.word	0
.text
.arm

_init:
				/* start main stack at top of internal RAM */
    			ldr   r0, =_eiram
    			msr   CPSR_c, #MODE_UND|I_BIT|F_BIT
    			mov   sp, r0
    			msr   CPSR_c, #MODE_ABT|I_BIT|F_BIT
    			mov   sp, r0
    			msr   CPSR_c, #MODE_FIQ|I_BIT|F_BIT
    			mov   sp, r0	
    			msr   CPSR_c, #MODE_IRQ|I_BIT|F_BIT
    			mov   sp, r0
    			msr   CPSR_c, #MODE_SVC|I_BIT|F_BIT
    			mov   sp, r0
    			sub   r0, r0, #8
    			msr   CPSR_c, #MODE_SYS|I_BIT|F_BIT
    			mov   sp, r0

				/* set up external memory */
				.set PINSEL2, 0xe002c014
				.set PINSEL2VAL, 0xf800924  /* Enable external bus, CS0, CS1 */

				ldr		r2, =PINSEL2
				ldr		r0, =PINSEL2VAL
				str		r0, [r2]

				.set BCFG0, 0xffe00000
				.set BCFG0VAL, 0x16002480  /* BCFG0: map external flash onto CS0 (0x80000000) */
				.set BCFG1VAL, 0x22000400 /* BCFG1: map external 10ns RAM onto CS1 (0x81000000) */
	
				ldr		r2, =BCFG0
				ldr		r0, =BCFG0VAL
				str		r0, [r2], #4
				ldr		r0, =BCFG1VAL
				str		r0, [r2]

				/* sync up with JTAGkey/gdb */
				ldr		r1, =_busy_flag
				mov		r0, #0
				strb	r0, [r1]
				bl		busy_wait
	
				/* copy .data section (Copy from ROM to RAM) */
                ldr     r1, =_etext
                ldr     r2, =_data
                ldr     r3, =_edata
1:        		cmp     r2, r3
                ldrlo   r0, [r1], #4
                strlo   r0, [r2], #4
                blo     1b

				/* Clear .bss section (Zero init)  */
                mov     r0, #0
                ldr     r1, =_bss_start
                ldr     r2, =_bss_end
2:				cmp     r1, r2
                strlo   r0, [r1], #4
                blo     2b

			
				/* Invoke constructors */
				ldr		r0, =_ctors_start_
				ldr		r1, =_ctors_end_
3:
				cmp		r0, r1
				beq		4f
				ldr		r2, [r0], #4
				stmfd	sp!, {r0-r1}
				mov		lr, pc
				mov		pc, r2
				ldmfd	sp!, {r0-r1}
				b		3b
4:		
				mov		fp, #0
				bl		hwinit
				bl		coreinit
                bl      main
				b		.		/* Wait for watchdog reset, if enabled */
	
.endfunc
.end
