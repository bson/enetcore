extern "C" {
	void IRQ_Routine ()   __attribute__ ((interrupt("IRQ")));
	void FIQ_Routine ()   __attribute__ ((interrupt("FIQ")));
	void SWI_Routine ()   __attribute__ ((interrupt("SWI")));
	void UNDEF_Routine () __attribute__ ((interrupt("UNDEF")));


	extern char _etext;
	extern volatile char _data;
}

#include "LPC22xx.h"


struct Test {
	char _c;
	Test(char c) : _c(c) { }
};

static Test test('A');


int	main () {
	
	int		j;										// loop counter (stack variable)
	
	extern void hwinit();
	hwinit();

	// Enable UART0, UART1 Rx/Tx pins
	PINSEL0 = 0x50005;

	// set io pins for led P0.7
	IO1DIR |= 0x00800000;	// pin P1.23 is an output, everything else is input after reset
	IO1SET =  0x00800000;	// led off
	IO1CLR =  0x00800000;	// led on

	// Baud clock =  cclk/(rate*16)
	// = 14755600*4/9600/16 = 0x180
	UART1_LCR = 0x83;		  // Enable access to divisor latch (8N1)
	UART1_DLL = 0x80;		  // Divisor latch least significant
	UART1_DLM = 0x1;		  // Divisor latch most significant
	UART1_LCR = 3;			  // Disable access to divisor latch (8N1)

	UART1_FCR = 7;			  // Reset and enable FIFOs

	char c = test._c;

	void* tmp = &_etext;

	volatile long* ptr = (volatile long*)0x81000000;
	*ptr = 0xdeadbeef;

	// endless loop to toggle the red  LED P1.23 and output a char
	while (1) {
		UART1_THR = c++;
		if (c == 'Z'+1)  c = 'a';
		else if (c == 'z'+1) c = 'A';

		IO1CLR = 0x00800000;
		for (j = 0; j < 200000; j++ );
		IO1SET = 0x00800000;
		for (j = 0; j < 2000000; j++ );
	}
}



void IRQ_Routine () {
	while (1) ;	
}

void FIQ_Routine ()  {
	while (1) ;	
}


void SWI_Routine ()  {
	while (1) ;	
}


void UNDEF_Routine () {
	while (1) ;	
}
