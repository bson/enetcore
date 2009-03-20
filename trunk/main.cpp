#include "enetkit.h"
#include "serial.h"
#include "util.h"
#include "thread.h"


Thread* test_thread;

void* ttfunc(void* tmp)
{
	DMSG("ttfunc: %p", tmp);
	for (;;) {
		DMSG("ttfunc");
		udelay(500000);
		Thread::Self().Yield(_main_thread);
	}
}


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.WriteSync(String(STR("\xfe\1Enetcore 0.1 DEV")));

	console("Enetcore 0.1 DEV");

	test_thread = new Thread(ttfunc, (void*)0xdeadbeef);

	for (;;) {
		if (_vic.ChannelPending(6)) {
			console("Unwedging!");
			_vic.ClearPending();
		}
		DMSG( "main");
		udelay(500000);
		Thread::Self().Yield(test_thread);
	}

	abort();
}
