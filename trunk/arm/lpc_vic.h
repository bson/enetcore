#ifndef __LPC_VIC_H__
#define __LPC_VIC_H__

// Vectored interrupt controller
class Vic {
	volatile uint32_t* const _base;
	mutable Spinlock _lock;
	uint _num_handlers;
public:
	Vic(uint32_t base) : _base((volatile uint32_t*) base), _num_handlers(0) { }
	
	void InstallHandler(uint channel, IRQHandler handler);
	void EnableChannel(uint channel);
	void DisableChannel(uint channel);

	// True if channel has a pending interrupt
	bool ChannelPending(uint channel);

	// Clear pending interrupt status - call prior to return
	void ClearPending();

private:
	static void Unhandled_IRQ() __irq;
};

#endif // __LPC_VIC_H__
