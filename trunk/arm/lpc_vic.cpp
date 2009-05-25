#include "enetkit.h"


void Vic::InstallHandler(uint channel, IRQHandler handler)
{
	Spinlock::Scoped L(_lock);

	if (channel == NOT_FOUND) {
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
