ENTRY(_startup)

MEMORY 
{
	flash     			: ORIGIN = 0,          LENGTH = 512K
	ram   				: ORIGIN = 0x10000000, LENGTH = 64K

    /* Peripheral SRAM */
    sram0               : ORIGIN = 0x20000000, LENGTH = 16K
    sram1               : ORIGIN = 0x20004000, LENGTH = 16K
}

_eiram  = 0x10000000 + 64K;

_sram0  = 0x20000000;
_esram0 = 0x10004000;

_sram1  = 0x20004000;
_esram1 = 0x20008000;

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
	
/* This should go in a separate file for LPC407x/8x */
PCONP = 0x400fc0c4;
PCONP1 = 0x400fc0c8;
PBOOST = 0x400fc1b0;
EMCCLKSEL = 0x400fc100;
CCLKSEL = 0x400fc104;
USBCLKSEL = 0x400fc108;
CLKSRCSEL = 0x400fc10c;
PCLKSEL = 0x400fc1a8;
SPIFICLKSEL = 0x400fc1b4;
EXTINT = 0x400fc140;
EXTMODE = 0x400fc148;
EXTPOLAR = 0x400fc14c;
RSID = 0x400fc180;
RSTCON0 = 0x400fc1cc;
RSTCON1 = 0x400fc1d0;
EMCDLYCTL = 0x400fc1dc;
EMCCAL = 0x400fc1e0;
SCS = 0x400fc1a0;
IRCCTRL = 0x400fc1a4;
LCD = 0x400fc1b8;
CANSLEEPCLR = 0x400fc110;
CANWAKEFLAGS = 0x400fc114;
USBINTST = 0x400fc1c0;
CLKOUTCFG = 0x400fc1c8;
FLASHCFG = 0x400fc000;
IOCON_P0 = 0x4002c000;
IOCON_P1 = 0x4002c080;
IOCON_P2 = 0x4002c100;
IOCON_P3 = 0x4002c180;
IOCON_P4 = 0x4002c200;
IOCON_P5 = 0x4002c280;
MATRIXARB = 0x400fc188;
MEMMAP = 0x400fc040;

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

