#ifndef __LPC_EINTR_H__
#define __LPC_EINTR_H__


// External LPC interrupt
// This can be used in one of two ways:
//
//   Pass it into a peripheral driver that accepts an LpcEint for its
//   interrupt abstraction.  It should keep a reference to this and
//   use it instead of accessing the VIC directly.  During init, the
//   driver's interrupt handler should be installed instead of the
//   one in Eintr.  The end of the handler should still clear pending
//   interrupts in the VIC before returning.
//
//   Install the LpcEintr interrupt handler and subclass it.
//   In this case, InterruptHandler() will be called for each
//   interrupt, from an IRQ/FIQ context.
//
// The Eintr needs to be initialized (via Init()) before use, or it
// will be dormant.
//

class LpcEintr {
	uint _intch:6;				// Interrupt channel
	uint _extint:4;				// EXTINT mask
	bool _wake:1;				// Wake on interrupt
	bool _low:1;				// Active low
	bool _level:1;				// Level triggered

public:
	enum { EINTR_WAKE = 1,		// Wake from sleep
		   EINTR_LOW = 2,		// Active low
		   EINTR_LEVEL = 4,		// Level triggered
		   EINTR_NOWAKE = 0,
		   EINTR_HIGH = 0,
		   EINTR_EDGE = 0
	};

	// Default is no wake, active low, edge triggered
	LpcEintr(uint num, uint flags = EINTR_LOW | EINTR_NOWAKE | EINTR_EDGE);

	// Enable EINTR.  Note: this function ONLY enables the interrupt,
	// it doesn't do any pin configuration.  Make sure the EINTR to be
	// enabled has the pin set up first. 
	void Init();

	// Use external handler.  Calls Init().
	void InstallHandler(IRQHandler handler);

	// Subclassing handler
	static void Interrupt() __irq __naked;

	// Interrupt handler
	virtual void HandleInterrupt() { }
	
	// Enable/disable interrupts
	void Enable(bool enable = true);

	// Check for interrupt
	bool Pending();

	// Clear pending
	void ClearPending();
};


#endif // __LPC_EINTR_H__
