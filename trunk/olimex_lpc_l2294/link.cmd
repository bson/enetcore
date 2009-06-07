ENTRY(_startup)

MEMORY 
{
	flash     			: ORIGIN = 0,          LENGTH = 256K
	ram_isp_low(A)		: ORIGIN = 0x40000120, LENGTH = 223
	ram   				: ORIGIN = 0x40000200, LENGTH = 15840
	ram_isp_high(A)		: ORIGIN = 0x40001ee0, LENGTH = 32
	xram				: ORIGIN = 0x81000000, LENGTH = 1M
	xflash				: ORIGIN = 0x80000000, LENGTH = 2M
}

_eiram = 0x40003edc;

SECTIONS 
{
	. = 0;						/* set location counter to address zero  */
	
	startup : {
		KEEP(*(.startup))
	} >flash		/* the startup code goes into FLASH */
	
	.text :						/* collect all sections that should go into FLASH after startup  */ 
	{
		*(.text)				/* all .text sections (code)  */
		*(.text*)				/* all .text sections (code)  */

		_ctors_start_ = .;
		KEEP(*(.ctors))
		KEEP(*(SORT(.ctors.*)))		/* Sort constructors */
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

	/* main thread stack is at top of internal RAM */
   .stack :
	{
		*(.iram)
		_iram = .;

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
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(4);						/* advance location counter to the next 32-bit boundary */
		_bss_end = . ;						/* define a global symbol marking the end of the .bss section */
	} >xram								/* put all the above in RAM (it will be cleared in the startup code */


	/* Define a section .xflash for external flash */
	.xflash :
	{
		_xflash = .;
		*(.xflash .xflash.*)
		_exflash = .;
	} >xflash
}
	
