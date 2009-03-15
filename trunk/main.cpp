extern "C" {
	void IRQ_Routine ()   __attribute__ ((interrupt("IRQ")));
	void FIQ_Routine ()   __attribute__ ((interrupt("FIQ"), naked));
	void SWI_Routine ()   __attribute__ ((interrupt("SWI")));
	void UNDEF_Routine () __attribute__ ((interrupt("UNDEF")));


	extern char _etext;
	extern volatile char _data;
}

typedef unsigned int uint;

#include "LPC22xx.h"


// We only mask IRQ, not FIQ

inline uint DisableInterrupts() {
	uint prev;
	asm volatile("mrs r12, %%cpsr_c\n"
				 "mov %0, r12\n"
				 "or r12, #0x80|0x40\n"
				 "msr %%cpsr_c, r12"
				 : "=r" (prev) : : "r12", "cc", "memory");
	return prev;
}

inline void EnableInterrupts(uint prev) {
	asm volatile("msr %%cpsr_c, %0" : : "X" (prev) : "cc", "memory");
}

void LedFlash(uint num)
{
	while (num--) {
		IO1CLR = 0x00800000;
		for (int j = 0; j < 100000; j++ ) continue;
		IO1SET = 0x00800000;
		if (num)
			for (int j = 0; j < 800000; j++ ) continue;
	}
}


void SetLedBrightness(uint num)
{
	PWM_MR1 = 1 << num;			// Update PWMMR1
	PWM_LER = 2;				// Latch in new PWMMR1
}



struct Test {
	char _c;
	Test(char c) : _c(c) { }
};

static Test test('A');


int	main ()
{
	extern void hwinit();
	hwinit();

	PINSEL0 = 0x50005;
	IO1DIR |= 0x00800000;	 // pin P1.23 is an output, everything else is input after reset
	IO1SET =  0x00800000;	 // led off
	IO1CLR =  0x00800000;	// led on

	// Enable interrupts
	asm volatile ("mrs r12, cpsr; bic r12, #0x40|0x80; msr cpsr, r12");


	// Baud clock =  cclk/(rate*16)
	// = 14755600*4/9600/16 = 0x180
	UART1_LCR = 0x83;			   // Enable access to divisor latch (8N1)
	UART1_DLL = 0x80;			   // Divisor latch least significant
	UART1_DLM = 0x1;			   // Divisor latch most significant
	UART1_LCR = 3;				   // Disable access to divisor latch (8N1)

	UART1_FCR = 7;				   // Reset and enable FIFOs

	char c = test._c;

	void* tmp = &_etext;

	// endless loop to toggle the red  LED P1.23 and output a char
	while (1) {
		UART1_THR = c++;
		if (c == 'Z'+1)  c = 'a';
		else if (c == 'z'+1) c = 'A';

		LedFlash(3);

		for (int j = 0; j < 6000000; j++ );
	}
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
