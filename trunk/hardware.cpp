#include "enetkit.h"

#include "lpc22xx.h"

#include "serial.h"
#include "timer.h"


extern "C" {
void Unexpected_Interrupt() __irq;

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


volatile uint8_t _busy_flag;

void busy_wait()
{
#ifdef BUSY_WAIT
	while (!_busy_flag) continue;
#endif
}


Vic _vic(VIC_BASE);


void Vic::Unhandled_IRQ()
{
	panic("Unhandled IRQ");
}


void Vic::InstallHandler(uint channel, IRQHandler handler)
{
	Spinlock::Scoped L(_lock);

	if (channel == (uint)-1) {
		_base[VIC_DefVectAddr] = (uint32_t)handler;
		return;
	}

	assert(channel < 32);

	// There are only 16 slots
	assert(_num_handlers < 16);

	_base[VIC_IntSelect] &= ~(1 << channel); // Make channel IRQ

	// Assign next available slot
	_base[VIC_VectCntl0 + _num_handlers] = (1 << 5) + channel;
	_base[VIC_VectAddr0 + _num_handlers] = (uint32_t)handler; // Vector
	++_num_handlers;
}


void Vic::EnableChannel(uint channel)
{
	assert(channel < 32);

	Spinlock::Scoped L(_lock);
	_base[VIC_IntEnable] |= 1 << channel;
}


void Vic::DisableChannel(uint channel)
{
	assert(channel < 32);

	Spinlock::Scoped L(_lock);
	_base[VIC_IntEnClr] = 1 << channel;
}


bool Vic::ChannelPending(uint channel)
{
	return (_base[VIC_IRQStatus] & (1 << channel)) != 0;
}


void Vic::ClearPending()
{
	// Clear pending interrupt by doing a dummy write to VICVectAddr
	_base[VIC_VectAddr] = _base[VIC_VectAddr];
}


void* _main_thread_stack;
void* _intr_thread_stack;


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

	// For now, map internal flash to vectors
	// 1 = flash, 2 = ram, 3 = xram
	MEMMAP = 1;
	
	// Disable watchdog
	WDMOD = 0;

	// Enable UARTs as output, along with pin P1.23 (LED)
	PINSEL0 = 0x50005;
	IO1DIR |= 0x00800000;	 // Make P1.23 an output, everything else stays input

	IO1SET =  0x00800000;	 // led off
	IO1CLR =  0x00800000;	// led on

	// Allocate main thread stack - this is the one we're using now
	// Since the region allocates low to high we do this by installing a reserve...
	_stack_region.SetReserve(MAIN_THREAD_STACK);
	asm volatile("mov %0, sp; add %0, #4" : "=r"(_main_thread_stack));

	// Allocate interrupt thread stack and install it
	_intr_thread_stack = _stack_region.GetMem(INTR_THREAD_STACK);
	
	asm volatile("mrs r2, cpsr; msr cpsr, #0x12|0x80|0x40; mov sp, %0; msr cpsr, r2"
				 : : "r"(_intr_thread_stack) : "cc", "r2", "memory");

	console("");
	DMSG("Main thread stack at (approx) %p; interrupt thread stack at %p",
		 _main_thread_stack, _intr_thread_stack);

	// Install IRQ handlers
	_vic.InstallHandler(6, SerialPort::Interrupt); // Channel 6 is UART0
	_vic.EnableChannel(6);
	_uart0.SetInterrupts(true);

	_vic.InstallHandler(4, Clock::Interrupt); // Channel 4 is TIMER0/Clock
	_vic.EnableChannel(4);

	// Enable interrupts
	asm volatile ("mrs r12, cpsr; bic r12, #0x40|0x80; msr cpsr, r12" : : : "r12", "cc", "memory");

	// Start clock
	_clock.SetResolution(TIME_RESOLUTION);
	_clock.RunTimer(HZ, true);
}


void feed()
{
	PLLFEED=0xAA;
	PLLFEED=0x55;
}


void Unexpected_Interrupt()  { /* for (;;) ; */ }
