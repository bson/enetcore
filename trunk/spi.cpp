#include "enetkit.h"
#include "spi.h"
#include "thread.h"


SPI _spi0(SPI0_BASE);


SPI::SPI(uint32_t base)
{
	_base = (volatile uint8_t*)base;
}


void SPI::Init()
{
	Deselect();
}


void SPI::SetSpeed(uint hz)
{
	Spinlock::Scoped L(_lock);

	Select();

	const uint scaler = PCLK/hz & ~1;
	DMSG("SPI: Prescaler = %u", max(min(scaler, (uint)254), (uint)8));
	_base[SPCCR] = max(min(scaler, (uint)254), (uint)8);

	// SPIE=1, LSBF=0, MSTR=1, CPOL=1, CPHA=0
	_base[SPCR] = 0b00110000;

	for (uint i = 0; i < 20; ++i) {
		_base[SPDR] = 0xff;
		while (!(_base[SPSR] & 0x80)) ;
	}

	_base[SPCR] = 0b10110000;
}


// * static __irq NAKED
void SPI::Interrupt()
{
	SaveStateExc(4);

	if (_vic.ChannelPending(10))
		_spi0.HandleInterrupt();

#if 0
	if (_vic.ChannelPending(11))
		_spi1.HandleInterrupt();
#endif

	_vic.ClearPending();

	LoadStateReturnExc();
}


void SPI::HandleInterrupt()
{
	Spinlock::Scoped L(_lock);

	if ((_base[SPSR] & 0b11111000) == 0x80) {
		if (_readbuf.Size() < _readbuf.GetReserve()) {
			const uint8_t c = _base[SPDR];
			// Ignore leading 0xff - it's just a non-transmitting slave
			if (!_readbuf.Empty() || c != 0xff) {
				_readbuf.PushBack(c);
				if (_readbuf.Size() == _read_count)
					Thread::WakeSingle(&_readbuf);
			}
		}

		if (!_writebuf.Empty()) {
			const uint8_t c = _writebuf.Front();
			_writebuf.PopFront();
			_base[SPDR] = c;
			if (_writebuf.Size() == _write_lowat)
				Thread::WakeSingle(&_writebuf);
		} else {
			if (_readbuf.Size() < _read_count)
				_base[SPDR] = 0xff;
		}
	} else {
		uint8_t c = _base[SPDR]; // Read to clear flags
	}

	// Clear interrupt flag
	_base[SPINT] = 1;
}


void SPI::Select()
{
	IO0CLR = 1 << 10;
}


void SPI::Deselect()
{
	IO0SET = 1 << 10;
}


void SPI::SetReadCount(uint hiwat)
{
	_lock.Lock();
	_read_count = hiwat;
	if (_readbuf.Size() >= hiwat)  {
		_lock.Unlock();
		Thread::Self().WakeAll(&_readbuf);
	} else
		_lock.Unlock();
}


void SPI::SetWriteMark(uint lowat)
{
	_lock.Lock();
	_write_lowat = lowat;
	if (_writebuf.Size() <= lowat)  {
		_lock.Unlock();
		Thread::Self().WakeAll(&_writebuf);
	} else
		_lock.Unlock();
}


bool SPI::WaitRead(Time deadline)
{
	Spinlock::Scoped L(_lock);
	while (_readbuf.Size() < _read_count && Time::Now() < deadline)  {
		_lock.Unlock();
		Thread::Self().WaitFor(&_readbuf, deadline);
		_lock.Lock();
	}
	return _readbuf.Size() >= _read_count;
}


bool SPI::WaitWrite(Time deadline)
{
	Spinlock::Scoped L(_lock);
	while (_writebuf.Size() > _write_lowat && Time::Now() < deadline)  {
		_lock.Unlock();
		Thread::Self().WaitFor(&_writebuf, deadline);
		_lock.Lock();
	}
	return _writebuf.Size() <= _write_lowat;
}


void SPI::SetReadBuf(uint new_size)
{
	Spinlock::Scoped L(_lock);
	_readbuf.Reserve(new_size);
}


void SPI::SetWriteBuf(uint new_size)
{
	Spinlock::Scoped L(_lock);
	_writebuf.Reserve(new_size);
}


void SPI::Send(const void* buf, uint numbytes)
{
	Spinlock::Scoped L(_lock);
	_writebuf.Erase(0, _writebuf.Size());
	assert(numbytes < _writebuf.GetReserve());
	_base[SPDR] = *((const uint8_t*&)buf)++;
	--numbytes;
	_writebuf.PushBack((const uint8_t*)buf, numbytes);
}


void SPI::Recv(void* buf, uint numbytes)
{
	Spinlock::Scoped L(_lock);
	memcpy(buf, _readbuf + 0, numbytes);
	_readbuf.Erase(0, numbytes);
	_readbuf.AutoCompact();
}
