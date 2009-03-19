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
		for (int j = 0; j < 6000000; j++ ) continue;

		Thread::Self().Yield(_main_thread);
	}
}


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.WriteSync(String(STR("\xfe\1Enetcore 0.1 DEV")));

	for (int j = 0; j < 100000; j++ ) continue;

	console("Enetcore 0.1 DEV");

	for (int j = 0; j < 100000; j++ ) continue;

	void* tmp = xmalloc(31);

	DMSG("Allocated %d bytes at %p", 31, tmp);

	test_thread = new Thread(ttfunc, (void*)0xdeadbeef);

	for (;;) {
		DMSG("main");
		for (int j = 0; j < 6000000; j++ ) continue;

		Thread::Self().Yield(test_thread);
	}

	abort();
}
