/*
The Philips boot loader supports the ISP (In System Programming) via the serial port and the IAP
(In Application Programming) for flash programming from within your application.

The boot loader uses RAM memory and we MUST NOT load variables or code in these areas.

RAM used by boot loader:  0x40000120 - 0x400001FF  (223 bytes) for ISP variables
                          0x4000FFE0 - 0x4000FFFF  (32 bytes)  for ISP and IAP variables
                          0x4000FEE0 - 0x4000FFDF  (256 bytes) stack for ISP and IAP
*/


/*
                           MEMORY MAP
                   |                                 |0x40010000
         .-------->|---------------------------------|
         .         |                                 |0x4000FFFF
      ram_isp_high |     variables and stack         |
         .         |     for Philips boot loader     |
         .         |         288 bytes               |
         .         |   Do not put anything here      |0x4000FEE0
         .-------->|---------------------------------|
                   |    UDF Stack  4 bytes           |0x4000FEDC  <---------- _stack_end
         .-------->|---------------------------------|
                   |    ABT Stack  4 bytes           |0x4000FED8
         .-------->|---------------------------------|
                   |    FIQ Stack  4 bytes           |0x4000FED4
         .-------->|---------------------------------|
                   |    IRQ Stack  4 bytes           |0x4000FED0
         .-------->|---------------------------------|
                   |    SVC Stack  4 bytes           |0x4000FECC
         .-------->|---------------------------------|
         .         |                                 |0x4000FEC8
         .         |     stack area for user program |
         .         |                                 |
         .         |                                 |
         .         |                                 |
         .         |                                 |
         .         |                                 |
         .         |                                 |
         .         |                                 |
         .         |          free ram               |
        ram        |                                 |
         .         |                                 |
         .         |                                 |
         .         |.................................|0x40000234 <---------- _bss_end
         .         |                                 |
         .         |  .bss   uninitialized variables |
         .         |.................................|0x40000218 <---------- _bss_start, _edata
         .         |                                 |
         .         |                                 |
         .         |                                 |
         .         |  .data  initialized variables   |
         .         |                                 |
         .         |                                 |
         .         |                                 |0x40000200 <---------- _data
         .-------->|---------------------------------|
         .         |     variables used by           |0x400001FF
      ram_isp_low  |     Philips boot loader         |
         .         |           223 bytes             |0x40000120
         .-------->|---------------------------------|
         .         |                                 |0x4000011F
      ram_vectors  |          free ram               |
         .         |---------------------------------|0x40000040
         .         |                                 |0x4000003F
         .         |  Interrupt Vectors (re-mapped)  |
         .         |          64 bytes               |0x40000000
         .-------->|---------------------------------|
                   |                                 |



                   |                                 |
        .--------> |---------------------------------|
        .          |                                 |0x0001FFFF
        .          |                                 |
        .          |                                 |
        .          |                                 |
        .          |                                 |
        .          |                                 |
        .          |       unused flash eprom        |
        .          |                                 |
        .          |.................................|
        .          |                                 |
        .          |                                 |
        .          |                                 |
        .          |      copy of .data area         |
      flash        |                                 |
        .          |                                 |
        .          |                                 |
        .          |---------------------------------|0x00000284 <----------- _etext
        .          |                                 |
        .          |                                 |0x00000180  main
        .          |                                 |0x00000104  Initialize
        .          |            C code               |0x00000100  UNDEF_Routine
        .          |                                 |0x000000fc  SWI_Routine
        .          |                                 |0x000000f8  FIQ_Routine
        .          |                                 |0x000000f4  IRQ_Routine
        .          |---------------------------------|0x000000d8  feed
        .          |                                 |0x000000D4
        .          |         Startup Code            |
        .          |         (assembler)             |
        .          |                                 |
        .          |---------------------------------|0x00000040 Reset_Handler
        .          |                                 |0x0000003F
        .          | Interrupt Vector Table (unused) |
        .          |          64 bytes               |
        .--------->|---------------------------------|0x00000000 _startup
*/


/* identify the Entry Point  */

ENTRY(_startup)



/* specify the LPC2106 memory areas  */

MEMORY 
{
	/* Internal flash */
	flash     			: ORIGIN = 0,          LENGTH = 256K

	/* variables used by Philips ISP bootloader	*/		 
	ram_isp_low(A)		: ORIGIN = 0x40000120, LENGTH = 223

	/* Internal RAM */
	ram   				: ORIGIN = 0x40000200, LENGTH = 15840

	/* variables used by Philips ISP bootloader	*/
	ram_isp_high(A)		: ORIGIN = 0x40001ee0, LENGTH = 32

	/* external RAM */
	xram				: ORIGIN = 0x81000000, LENGTH = 1M

    /* external FLASH ROM */
	xflash				: ORIGIN = 0x80000000, LENGTH = 2M
}



/* define a global symbol _stack_end  */

_estack = 0x40003edc;


/* now define the output sections  */

SECTIONS 
{
	. = 0;						/* set location counter to address zero  */
	
	startup : { *(.startup)} >flash		/* the startup code goes into FLASH */
	
	.text :						/* collect all sections that should go into FLASH after startup  */ 
	{
		*(.text)				/* all .text sections (code)  */
		*(.text*)				/* all .text sections (code)  */

		_ctors_start_ = .;
		*(.ctors)
		*(SORT(.ctors.*))		/* Sort constructors */
		_ctors_end_ = .;

		_dtors_start_ = .;
/*
No need to keep dtors if main() never returns
		
		*(.dtors)
		*(SORT(.dtors.*))
*/
		_dtors_end_ = .;

		*(.rodata)					/* all .rodata sections (constants, strings, etc.)  */
		*(.rodata*)					/* all .rodata* sections (constants, strings, etc.)  */

/* We don't use thumb mode, so no need for interwork glue 
		*(.glue_7)
		*(.glue_7t) */
		_etext = .;					/* define a global symbol _etext just after the last code byte */
	} >flash							/* put all the above into FLASH */

   .stack :
	{
		*(.iram)
		_stack = .;

	} >ram

	.data :								/* collect all initialized .data sections that go into RAM  */ 
	{
		_data = .;						/* create a global symbol marking the start of the .data section  */
		*(.data)						/* all .data sections  */
		_edata = .;						/* define a global symbol marking the end of the .data section  */
	} >xram AT >flash					/* put all the above into external RAM (but load the LMA copy into FLASH) */

	.bss :								/* collect all uninitialized .bss sections that go into RAM  */
	{
		_bss_start = .;					/* define a global symbol marking the start of the .bss section */
		*(.bss)							/* all .bss sections  */
			. = ALIGN(4);						/* advance location counter to the next 32-bit boundary */
		_bss_end = . ;						/* define a global symbol marking the end of the .bss section */
	} >xram								/* put all the above in RAM (it will be cleared in the startup code */


	.xflash :
	{
		_xflash = .;
		*(.xflash)
		_exflash = .;
	} >xflash
}
	
