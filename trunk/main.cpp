#include "enetkit.h"
#include "serial.h"
#include "util.h"
#include "thread.h"
#include "mutex.h"


Mutex mtx;
CondVar cv;
int runner;

Thread* test_thread;

void* ttfunc(void* tmp)
{
	DMSG("ttfunc: %p", tmp);
	for (;;) {
		Mutex::Scoped L(mtx);
		while (!runner)
			cv.Wait(mtx);

		DMSG("ttfunc");
//		udelay(500000);
		Thread::Self().Delay(500000);
		runner = !runner;
		cv.Signal();

//		Thread::Self().Yield(_main_thread);
	}
}


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.WriteSync(String(STR("\xfe\1Enetcore 0.1 DEV")));

	console("Enetcore 0.1 DEV");

	test_thread = new Thread(ttfunc, (void*)0xdeadbeef);

	runner = 0;

	for (;;) {
		Mutex::Scoped L(mtx);
		while (runner)
			cv.Wait(mtx);

		DMSG( "main");
		udelay(500000);
		// Thread::Self().Delay(500000);
		runner = !runner;
		cv.Signal();

//		Thread::Self().Yield(test_thread);
	}

	abort();
}
