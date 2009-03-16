
/* Stack Sizes */
.set  UND_STACK_SIZE, 0x00000004		/* stack for "undefined instruction" interrupts is 4 bytes  */
.set  ABT_STACK_SIZE, 0x00000004		/* stack for "abort" interrupts is 4 bytes                  */
.set  FIQ_STACK_SIZE, 0x00000004		/* stack for "FIQ" interrupts  is 4 bytes         			*/
.set  IRQ_STACK_SIZE, 0X00000004		/* stack for "IRQ" normal interrupts is 4 bytes    			*/
.set  SVC_STACK_SIZE, 0x00000004		/* stack for "SVC" supervisor mode is 4 bytes  				*/



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

.set BCFG0, 0xffe00000

.set PINSEL2, 0xe002c014
	
/* Enable external bus, CS0, CS1 */
.set PINSEL2VAL, 0xf800924

/* BCFG0 command to map external flash onto CS0 (0x80000000) */
.set BCFG0VAL, 0x16002480

/* BCFG1: map external 10ns RAM onto CS1 (0x81000000) */
.set BCFG1VAL, 0x22000400
.text
.arm

.global	Reset_Handler
.global _startup
.func   _startup

_startup:

# Exception Vectors

_vectors:       ldr     PC, Reset_Addr         
                ldr     PC, Undef_Addr
                ldr     PC, SWI_Addr
                ldr     PC, PAbt_Addr
                ldr     PC, DAbt_Addr
                nop							/* Reserved Vector (holds Philips ISP checksum) */
                ldr     PC, [PC,#-0xFF0]	/* see page 71 of "Insiders Guide to the Philips ARM7-Based Microcontrollers" by Trevor Martin  */
                ldr     PC, FIQ_Addr

Reset_Addr:     .word   Reset_Handler		/* defined in this module below  */
Undef_Addr:     .word   UNDEF_Routine		/* defined in main.c  */
SWI_Addr:       .word   SWI_Routine			/* defined in main.c  */
PAbt_Addr:      .word   UNDEF_Routine		/* defined in main.c  */
DAbt_Addr:      .word   UNDEF_Routine		/* defined in main.c  */
IRQ_Addr:       .word   IRQ_Routine			/* defined in main.c  */
FIQ_Addr:       .word   FIQ_Routine			/* defined in main.c  */
                .word   0					/* rounds the vectors and ISR addresses to 64 bytes total  */


# Reset Handler

Reset_Handler:  

				/* Setup a stack for each mode - note that this only sets up a usable stack
				for User mode.   Also each mode is setup with interrupts initially disabled. */
    			  
    			ldr   r0, =_estack
    			msr   CPSR_c, #MODE_UND|I_BIT|F_BIT 	/* Undefined Instruction Mode  */
    			mov   sp, r0
    			sub   r0, r0, #UND_STACK_SIZE
    			msr   CPSR_c, #MODE_ABT|I_BIT|F_BIT 	/* Abort Mode */
    			mov   sp, r0
    			sub   r0, r0, #ABT_STACK_SIZE
    			msr   CPSR_c, #MODE_FIQ|I_BIT|F_BIT 	/* FIQ Mode */
    			mov   sp, r0	
   				sub   r0, r0, #FIQ_STACK_SIZE
    			msr   CPSR_c, #MODE_IRQ|I_BIT|F_BIT 	/* IRQ Mode */
    			mov   sp, r0
    			sub   r0, r0, #IRQ_STACK_SIZE
    			msr   CPSR_c, #MODE_SVC|I_BIT|F_BIT 	/* Supervisor Mode */
    			mov   sp, r0
    			sub   r0, r0, #SVC_STACK_SIZE
    			msr   CPSR_c, #MODE_SYS|I_BIT|F_BIT 	/* User Mode */
    			mov   sp, r0

				/* set up external memory */
				ldr		r0, =PINSEL2VAL
				ldr		r2, =PINSEL2
				str		r0, [r2]

				ldr		r2, =BCFG0
				ldr		r0, =BCFG0VAL
				str		r0, [r2], #4
				ldr		r0, =BCFG1VAL
				str		r0, [r2]

				/* sync up with JTAGkey/gdb */
				mov		r0, #0
				ldr		r1, =_data
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
				/* Enter the C code  */
				bl		hwinit
                b       main

.endfunc
.end
