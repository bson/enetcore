ENTRY(_startup)

MEMORY 
{
	flash     			: ORIGIN = 0x08000000, LENGTH = 1024K
	ram   				: ORIGIN = 0x20000000, LENGTH = 128K
    ccmram              : ORIGIN = 0x10000000, LENGTH = 64K
}

_eiram  = 0x20000000 + 128K;

/* Symbol needed by NVic for stack vector */
__stack_top = ORIGIN(ccmram) + LENGTH(ccmram);

SECTIONS
{
	. = ORIGIN(flash);
	
	.text :
	{
		KEEP(*(.startup))
		*(.text)
		*(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

		_ctors_start_ = .;
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
		KEEP(*(SORT(.ctors.*)))		/* Sort constructors */
		KEEP(*(.ctors))
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

/* We only use thumb2 mode, do we need this glue? */
/*		*(.glue_7)
		*(.glue_7t)
*/
		KEEP(*(.eh_frame*))

		. = ALIGN(4);
		/* preinit data */
		__preinit_array_start = .;
		KEEP(*(.preinit_array))
		__preinit_array_end = .;

		. = ALIGN(4);
		/* init data */
		__init_array_start = .;
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		__init_array_end = .;

		. = ALIGN(4);
		/* finit data */
		__fini_array_start = .;
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		__fini_array_end = .;

	} >flash							/* put all the above into FLASH */

    .ARM.extab :
    {
            *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > flash

    __exidx_start = .;
    .ARM.exidx : {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } >flash

    __exidx_end = .;

    . = ALIGN (4);
    _etext = .;

    .copy.table :
    {
            . = ALIGN(4);
            __copy_table_start = .;
            LONG (_etext)
            LONG (_data)
            LONG (_edata - _data)
            __copy_table_end = .;
    } > flash

    . = ORIGIN(ccmram);

	.data :
	{
        __vector_table = .;
        . = . + 0x400;

		_data = .;
		*(vtable)
		*(.data*)

        KEEP(*(.jcr*))

		. = ALIGN(4);
		_edata = .;
	} >ccmram

	.bss : {
		_bss_start = .;
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(4);
		_bss_end = . ;
	} >ccmram

	/* main thread stack is at top of internal RAM */
    /* There shouldn't be anything here */

   .stack (COPY):
	{
        *(.stack)
	} >ccmram
}

INCLUDE enetcore/arch/armv7m/cortex-m4.link.cmd
