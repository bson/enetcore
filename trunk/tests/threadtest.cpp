#include "enetkit.h"
#include "serial.h"
#include "util.h"
#include "thread.h"
#include "mutex.h"


Mutex mtx;
CondVar cv;
int runner;
EventObject ev1, ev2;

Thread* test_thread;

void* ttfunc(void* tmp)
{
	DMSG("ttfunc: %p", tmp);
	for (;;) {
//		ev1.Wait();

//		Mutex::Scoped L(mtx);
//		while (!runner)
//			cv.Wait(mtx);

		DMSG("ttfunc");
		for (uint i = 0; i < 3000000; ++i) ;

//		mtx.Lock();
//		cv.Wait(mtx, Time::FromMsec(500));
//		mtx.Unlock();

//		udelay(500000);
//		Thread::Self().Delay(500000);
//		runner = !runner;
//		cv.Signal();
//		ev2.Set();

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

//	ev2.Set();
	for (;;) {
//		Mutex::Scoped L(mtx);
//		while (runner)
//			cv.Wait(mtx);

//		ev2.Wait();

		DMSG( "main");

		for (uint i = 0; i < 3000000; ++i) ;
//		mtx.Lock();
//		cv.Wait(mtx, Time::FromMsec(500));
//		mtx.Unlock();

//		udelay(500000);
//		Thread::Self().Delay(500000);
//		runner = !runner;
//		cv.Signal();
//		ev1.Set();

//		Thread::Self().Yield(test_thread);
	}

	abort();
}
