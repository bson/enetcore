#include "enetkit.h"

#include "lpc22xx.h"


extern "C" {
void IRQ_Routine ()   __attribute__ ((interrupt("IRQ")));
void FIQ_Routine ()   __attribute__ ((interrupt("FIQ"), naked));
void SWI_Routine ()   __attribute__ ((interrupt("SWI")));
void UNDEF_Routine () __attribute__ ((interrupt("UNDEF")));

void feed();
void busy_wait () __attribute__ ((naked));

}


void fault0(uint num)
{
	for (uint n = num; n; --n) {
		IO1CLR = 0x00800000;
		for (int j = 0; j < 100000; j++ ) continue;
		IO1SET = 0x00800000;
		if (num)
			for (int j = 0; j < 800000; j++ ) continue;
	}

	for (int j = 0; j < 6000000; j++ );
}


void busy_wait()
{
#ifdef BUSY_WAIT
	while (!_data) continue;
#endif
}


#define PLOCK 0x400

void hwinit()
{
	
 
	// 				Setting the Phased Lock Loop (PLL)
	//               ----------------------------------
	//
	// Olimex LPC-P2106 has a 14.7456 mhz crystal
	//
	// We'd like the LPC2106 to run at 53.2368 mhz (has to be an even multiple of crystal)
	// 
	// According to the Philips LPC2106 manual:   M = cclk / Fosc	where:	M    = PLL multiplier (bits 0-4 of PLLCFG)
	//																		cclk = 53236800 hz
	//																		Fosc = 14745600 hz
	//
	// Solving:	M = 53236800 / 14745600 = 3.6103515625
	//			M = 4 (round up)
	//
	//			Note: M - 1 must be entered into bits 0-4 of PLLCFG (assign 3 to these bits)
	//
	//
	// The Current Controlled Oscilator (CCO) must operate in the range 156 mhz to 320 mhz
	//
	// According to the Philips LPC2106 manual:	Fcco = cclk * 2 * P    where:	Fcco = CCO frequency 
	//																			cclk = 53236800 hz
	//																			P = PLL divisor (bits 5-6 of PLLCFG)
	//
	// Solving:	Fcco = 53236800 * 2 * P
	//			P = 2  (trial value)
	//			Fcco = 53236800 * 2 * 2
	//			Fcc0 = 212947200 hz    (good choice for P since it's within the 156 mhz to 320 mhz range
	//
	// From Table 19 (page 48) of Philips LPC2106 manual    P = 2, PLLCFG bits 5-6 = 1  (assign 1 to these bits)
	//
	// Finally:      PLLCFG = 0  01  00011  =  0x23
	//
	// Final note: to load PLLCFG register, we must use the 0xAA followed 0x55 write sequence to the PLLFEED register
	//             this is done in the short function feed() below
	//
   
	// Setting Multiplier and Divider values
  	PLLCFG=0x23;
  	feed();
  
	// Enabling the PLL
	PLLCON=0x1;
	feed();
  
	// Wait for the PLL to lock to set frequency
	while(!(PLLSTAT & PLOCK)) ;
  
	// Connect the PLL as the clock source
	PLLCON=0x3;
	feed();
  
	// Enabling MAM and setting number of clocks used for Flash memory fetch (4 cclks in this case)
	MAMCR=0x2;
	MAMTIM=0x4;
  
	// Setting peripheral Clock (pclk) to System Clock (cclk)
	VPBDIV=0x1;

	// This is done in crt.s - it needs to be done before calling C code
#if 0
	// Enable use of external banks 0, 1
	// P3.26: /CS1
	// P3.27: /WE

	// These are controlled through bits 4-5; the standard philips
	// boot ROM initializes these from two jumpers.  We could do the same...
	//
	// P2.0-7: D0-D7
	// P1.0: /CS0
	// P1.1: /OE
	// P3.31: BLS0
	// P2.8-15: D8-D15
	// P3.30: BLS1
	// P2.16-27: D16-D27
	// P2.28-29: D28-29
	// P2.30-31: D30-31
	// P3.28-29: BLS2-3
	//
	// P3.0: A0
	// P3.1: A1

	// Enable debug (JTAG) port.  The Olimex L2294 board has a DBG jumper that
	// can be used for this...
	// Disable trace port.
	//
	// A2-A23 are used for address lines
	//
	// 0000 111 1 1 0 0 0 00 00 00 0 0 1 00 1 0 0 10 0 1 00
	// 0000 1111 1000 0000 0000 1001 0010 0100
	PINSEL2 = 0xf800924;
#endif

	// Map external 32-bit RAM at 0x8100 0000
	//
	// t_cyc = 1/(4*14745600) = ~17ns
	//
	// WST1 >= ((t_read + 20ns) / t_cyc) - 2
	// For 10ns SRAM, WST1 >= (10ns+20ns)/17ns - 2 >= 1.7 - 2 >= 0
	// WST2 >= (t_write - t_cyc + 5) / t_cyc
	// For 10ns SRAM, WST2 >= (10ns - 17ns + 5) / 17 >= 0
	//
	// But for safety, we set WST1, WST2 = 1
	// WST1 = 1, WST2 = 1
	// IDCY = 0 (no switch overhead)
	//
#define BCFGVAL(IDCY, WST1, WST2, WP, BM, MW) \
	(((MW) << 28) | ((BM) << 27) | ((WP) << 26) | ((WST2) << 11) | (1 << 10) | \
	 ((WST1) << 5) | (IDCY) | (1 << 25))

#if 0
	BCFG1 = BCFGVAL(0, 0, 0, 0, 0, 2);

	// Map external 16-bit flash at 0x8000 0000
	//
	// 70ns device
	// WST1 >= (70+20)/17 - 2 >= 3.29
	// WST2 >= (70-17+5)/17 >= 3.4
	//
	// WST1=4, WST2=4
	// IDCY=0
	// Write protect
	// Non-burst ROM
	//
	BCFG0 = BCFGVAL(0, 4, 4, 1, 0, 1);
#endif

	// Fully enable MAM
	MAMCR = 0;
	MAMTIM = 3;
	MAMCR = 2;

	// Turn off WP on external flash
	BCFG0 = BCFGVAL(0, 4, 4, 0, 0, 1);

	// For now, map flash to vectors
	// 1 = flash, 2 = ram, 3 = xram
	MEMMAP = 1;
	
	// Disable watchdog
	WDMOD = 0;

	// Enable UARTs as output, along with pin P1.23 (LED)
	PINSEL0 = 0x50005;
	IO1DIR |= 0x00800000;	 // Make P1.23 an output, everything else stays input

	IO1SET =  0x00800000;	 // led off
	IO1CLR =  0x00800000;	// led on

	// Enable interrupts
	asm volatile ("mrs r12, cpsr; bic r12, #0x40|0x80; msr cpsr, r12");
}


void feed()
{
	PLLFEED=0xAA;
	PLLFEED=0x55;
}


void IRQ_Routine () {
	while (1) ;	
	asm volatile ("subs pc, lr, #4");
}


void FIQ_Routine ()
{
	while (1) ;
	asm volatile ("subs pc, lr, #4");
}

void SWI_Routine ()  {
	while (1) ;	
}


void UNDEF_Routine () {
	while (1) ;	
}
