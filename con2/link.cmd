ENTRY(_startup)

MEMORY 
{
	flash     			: ORIGIN = 0x08000000, LENGTH = 1024K
	ram   				: ORIGIN = 0x20000000, LENGTH = 128K
}

_eiram  = 0x20000000 + 128K;

__stack_top = ORIGIN(ram) + LENGTH(ram);

SECTIONS 
{
	. = 0;
	
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

    _iram = .;

	/* main thread stack is at top of internal RAM */
    /* There shouldn't be anything here */

   .stack (COPY):
	{
        *(.stack)
	} >ram
}
	
/* ARM Core Cortex-M4 system control */
CPUID = 0xe000ed00;
ACTLR = 0xe000e008;
ICSR = 0xe000ed04;
VTOR = 0xe000ed08;
AIRCR = 0xe000ed0c;
SCR = 0xe000ed10;
CCR = 0xe000ed14;
SHPR1 = 0xe000ed18;
SHPR2 = 0xe000ed1c;
SHPR3 = 0xe000ed20;
SHCRS = 0xe000ed24;
CFSR = 0xe000ed28;
MMFSR = 0xe000ed28;
BFSR = 0xe000ed29;
UFSR = 0xe000ed2a;
HFSR = 0xe000ed2c;
MMAR = 0xe000ed34;
BFAR = 0xe000ed38;
AFSR = 0xe000ed3c;

/* ARM Core NVIC */
ISER0 = 0xe000e100;
ISER1 = 0xe000e104;
ICER0 = 0xe000e180;
ICER1 = 0xe000e184;
ISPR0 = 0xe000e200;
ISPR1 = 0xe000e204;
ICPR0 = 0xe000e280;
ICPR1 = 0xe000e284;
IABR0 = 0xe000e300;
IABR1 = 0xe000e304;
IPR0 = 0xe000e400;
IPR1 = 0xe000e404;
IPR2 = 0xe000e408;
IPR3 = 0xe000e40c;
IPR4 = 0xe000e410;
IPR5 = 0xe000e414;
IPR6 = 0xe000e418;
IPR7 = 0xe000e41c;
IPR8 = 0xe000e420;
IPR9 = 0xe000e424;
IPR10 = 0xe000e428;
STIR = 0xe000ef00;

/* ARM Core MPU */
MPU_TYPE = 0xe000ed90;
MPU_CTRL =  0xe000ed94;
MPU_RNR =  0xe000ed98;
MPU_RBAR =  0xe000ed9c;
MPU_RASR =  0xe000eda0;
MPU_RBAR_A1 =  0xe000eda4;
MPU_RASR_A1 = 0xe000eda8;
MPU_RBAR_A2 = 0xe000edac;
MPU_RASR_A2 = 0xe000edb0;
MPU_RBAR_A3 = 0xe000edb4;
MPU_RASR_A3 = 0xe000edb8;

/* ARM Core SYSTICK */

SYST_CSR = 0xe000e010;
SYST_RVR = 0xe000e014;
SYST_CVR = 0xe000e018;
SYST_CALIB = 0xe000e01c;

