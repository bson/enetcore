ENTRY(_startup)

INCLUDE enetcore/soc/stm327hx/h753.link.cmd

/* 
 * the above includes
 *    enetcore/arch/armv7m/cortex-m7.link.cmd 
*/

MEMORY 
{
	flash (rx!w)	: ORIGIN = 0x08000000, LENGTH = 1024K
	ram (rw)		: ORIGIN = 0x20000000, LENGTH = 128K
    ccmram (rw)     : ORIGIN = 0x10000000, LENGTH = 64K
}

_eiram  = ORIGIN(ram) + LENGTH(ram);

/* Symbol needed by NVic for stack vector */
__stack_top = ORIGIN(ram) + LENGTH(ram);

SECTIONS
{
	.text ORIGIN(flash) (READONLY) :
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

		KEEP(*(.eh_frame*))     /* XXX do we really need this? */

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

        *(.ARM.extab* .gnu.linkonce.armextab.*)

        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;


        . = ALIGN (4);
        __copy_table_start = .;
        LONG (_etext)
        LONG (_data)
        LONG (_edata - _data)
        __copy_table_end = .;

        _etext = .;

	} >flash = 0xff							/* put all the above into FLASH */

    . = ORIGIN(ram);

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
	} >ram

	.bss : {
		_bss_start = .;
		*(.dynbss)
		*(.bss .bss.* .gnu.linkonce.b.*)
		*(COMMON)
		. = ALIGN(4);
		_bss_end = . ;
	} >ram

   .stack (COPY):
	{
        *(.stack)
	} >ram
}
