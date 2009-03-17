#include "enetkit.h"
#include "serial.h"
#include "util.h"


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.WriteSync(String(STR("\xfe\1Enetcore 0.1 DEV")));

	console("Enetcore 0.1 DEV");

	void* tmp = xmalloc(31);

	DMSG("Allocated %d bytes at %p", 31, tmp);

	abort();
}
