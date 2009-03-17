#include "enetkit.h"
#include "serial.h"
#include "util.h"


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.WriteSync(String(STR("\xfe\1Enetcore 0.1 DEV")));

	for (int j = 0; j < 100000; j++ ) continue;

	console("Enetcore 0.1 DEV");

	for (int j = 0; j < 100000; j++ ) continue;

	void* tmp = xmalloc(31);

	DMSG("Allocated %d bytes at %p", 31, tmp);

	abort();
}
