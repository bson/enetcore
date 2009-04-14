#include "enetkit.h"

#include "lpc22xx.h"

#include "serial.h"
#include "timer.h"
#include "thread.h"
#include "spi.h"
#include "network.h"
#include "ethernet.h"
#include "util.h"


Thread* _main_thread;
void* _main_thread_stack;
void* _intr_thread_stack;


extern "C" {
void Unexpected_Interrupt() __irq NAKED;
void Data_Abort_Exception() __abort NAKED;
void Program_Abort_Exception() __abort NAKED;
void Undef_Exception() __undef NAKED;
void SWI_Trap() __swi NAKED;

void feed();
void busy_wait () NAKED;

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
	asm volatile ("mov pc, lr");
}


Vic _vic(VIC_BASE);


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
	_base[VIC_IntEnable] = 1 << channel;
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
	_base[VIC_VectAddr] = 0;
}


#define PLOCK 0x400

void hwinit()
{
	// Setting Multiplier and Divider values
  	PLLCFG=0x23;
  	feed();
  
	// Enabling the PLL
	PLLCON=0x1;
	feed();
  
	// Wait for the PLL to lock to set frequency
	while (!(PLLSTAT & PLOCK)) continue;
  
	// Connect the PLL as the clock source
	PLLCON=0x3;
	feed();
  
	// Setting peripheral Clock (pclk) to System Clock (cclk)
	VPBDIV=0x1;

	// This is done in crt.s - it needs to be done before calling C code
#if 0
	// Enable use of external banks 0, 1
	// P3.26: /CS1
	// P3.27: /WE
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
#define BCFGVAL(IDCY, WST1, WST2, WP, BM, MW, RBLE)							\
	(((MW) << 28) | ((BM) << 27) | ((WP) << 26) | ((WST2) << 11) | ((RBLE) << 10) | \
	 ((WST1) << 5) | (IDCY) | (1 << 25))

#if 0
	BCFG1 = BCFGVAL(0, 0, 0, 0, 0, 2, 1);

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
	BCFG0 = BCFGVAL(0, 4, 4, 1, 0, 1, 1);
#endif

	// Fully enable MAM
	MAMCR = 0;
	MAMTIM = 3;
	MAMCR = 2;

	// 1 = flash, 2 = ram, 3 = xram
	MEMMAP = 1;

	// Disable watchdog
	WDMOD = 0;

	// CS8900A: on CS2, 16 bit with BSEL
	// 135ns read, 110ns write
	// WST1 >= (135+20)/17 - 2 >= 7.11 (8)
	// WST2 >= (110-17+5)/17 >= 5.76 (6)
	BCFG2 = BCFGVAL(0, 8, 6, 0, 0, 1, 1);

	// Enable UARTs as output, SPI0 on pins
	PINSEL0 = 0b00000000000001010101010100000101;

	// Enable CS2 on P3.25
	PINSEL2 = (PINSEL2 & ~(0b11 << 14)) | (0b01 << 14);

	IO1DIR |= 0x00800000;	 // Make P1.23 an output, everything else stays input

	IO1SET =  0x00800000;	 // led off
	IO1CLR =  0x00800000;	// led on

	IO0DIR |= 0x80;				   // GPIO 0.7 is output (soft SSEL)
	IO0DIR |= 0x40 | 0x10;		   // GPIO 0.4 (SCK), 0.6 (MOSI) are out
	IO0DIR &= ~0x20;			   // GPIO 0.5 (MISO) is in
	IO0DIR |= 1 << 10;			   // GPIO 0.10 is output (CARD_CS)

	// Enable SPI1, AIN0
	PINSEL1 = 0b00000000001000000000000010101000;
	IO1DIR |= 0b1101 << 17;		// SCK1, MOSI1, SSEL1 are out
	IO1DIR &= ~(0b0010 << 17);		// MISO1 is in

	// Grab 32 bytes from AIN0
	uint8_t buf[32];
	for (uint8_t* p = (uint8_t*)buf; p < (uint8_t*)buf + sizeof buf; ) {
		// Does the first of these two actually have to be inside the loop?
		ADCR = 0x00200000 | (((PCLK / 4500000) + 1) << 8);
		ADCR |= 0x01000000;		// Go
		uint32_t ad;
		do {
			ad = ADDR;
		} while (!(ad & 0x80000000));
		*p++ = (ad & 0xffff) >> 6;
	}
	ADCR = 0;					// Power down ADC

	Util::RandomSeed(buf, sizeof buf);

	// Allocate main thread stack - this is the one we're using now
	// Since the region allocates low to high we do this by installing a reserve...
	_stack_region.SetReserve(MAIN_THREAD_STACK);
	_main_thread_stack = (uint8_t*)_stack_region.GetEnd() - MAIN_THREAD_STACK;
	// This is picked up by Thread::Initialize to wrap the current thread as the main thread

	// Allocate interrupt thread stack and install it
	// Allocate and install FIQ stack
	_intr_thread_stack = _stack_region.GetMem(INTR_THREAD_STACK);

	uint8_t* fiq_stack = (uint8_t*)_stack_region.GetMem(16);

	asm volatile("mrs r2, cpsr; "
				 "msr cpsr, #0x12|0x80|0x40; mov sp, %0;"
				 "msr cpsr, #0x11|0x80|0x40; mov sp, %1;"
				 "msr cpsr, r2"
				 : : "r"((uint8_t*)_intr_thread_stack + INTR_THREAD_STACK),
				   "r"(fiq_stack + 16) : "cc", "r2", "memory");

	// Install IRQ handlers
	_vic.InstallHandler(4, Clock::Interrupt); // Channel 4 is TIMER0/Clock
	_vic.EnableChannel(4);

	_vic.InstallHandler(5, SysTimer::Interrupt); // Channel 5 is TIMER1/SysTimer
	_vic.EnableChannel(5);

	_vic.InstallHandler(6, SerialPort::Interrupt); // Channel 6 is UART0
	_vic.EnableChannel(6);
	_uart0.SetInterrupts(true);
	_uart0.SetSpeed(115200);
	_uart0.Write(STR("\r\n\n"));

	_vic.InstallHandler(16, Ethernet::Interrupt);

	// Enable interrupts
	asm volatile ("mrs r12, cpsr; bic r12, #0x40|0x80; msr cpsr, r12" : : : "r12", "cc", "memory");

	// Start clock
	// Initialize threads
	_main_thread = &Thread::Initialize();

	_clock.SetResolution(TIME_RESOLUTION);
	_clock.RunTimerFreq(HZ, 0);

	console("Enetcore 0.1 DEV");

	void *sp;
	asm volatile("mov %0, sp" : "=r" (sp) : : "memory");
	DMSG("Main thread stack at (approx) %p (sp=%p); interrupt thread stack at %p",
		 _main_thread_stack, sp, _intr_thread_stack);

	DMSG("Random uint: 0x%x", Util::Random<uint>());

	_eth0.Initialize();

	// Set up external interrupt pins - do this last so vectors are ready

	// P0.15 = EINT2
	PINSEL0 = (PINSEL0 & ~(0b11 << 30)) | (0b10 << 30);
	EXTWAKE = 4;			   // EINT2 wakes from power-down
	VPBDIV=0;				   // Workaround for EXTINT.1, EXTINT.2 errata
	EXTPOLAR = 4;			   // Make EINT2 active high
	VPBDIV=1;				   // CPU bug workaround
	VPBDIV=0;				   // CPU bug workaround
	EXTMODE = 0;			   // Make EINT2 level triggered
	VPBDIV=1;				   // CPU bug workaround
	VPBDIV=1;				   // CPU bug workaround
	EXTINT = 4;				   // Clear any stray EINT2 flag
	_vic.ClearPending();
	_vic.EnableChannel(16);

//	_net_thread = new Thread(NetThread, NULL, NET_THREAD_STACK);
}


void feed()
{
	PLLFEED=0xAA;
	PLLFEED=0x55;
}


void Unexpected_Interrupt()  {
	console("Unhandled interrupt");
	_vic.ClearPending();
}

void Data_Abort_Exception()
{
	SaveStateExc(8);			// Set LR to point to aborted instruction
	Thread::Exception(Thread::DATA_ABORT);
}

void Program_Abort_Exception()
{
	SaveStateExc(8);			// Set LR to point to aborted instruction
	Thread::Exception(Thread::PROGRAM_ABORT);
}

void Undef_Exception()
{
	SaveStateExc(4);			// Set LR to point to undefined instruction
	Thread::Exception(Thread::UNDEF);
}

void SWI_Trap()
{
	SaveStateExc(4);			// Set LR to point to SWI instruction
	Thread::Exception(Thread::SWI);
}
