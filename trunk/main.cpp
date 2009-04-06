#include "enetkit.h"
#include "serial.h"
#include "util.h"
#include "sdcard.h"


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.WriteSync(String(STR("\xfe\1Enetcore 0.1 DEV")));

	console("Enetcore 0.1 DEV");

	_sd.Init();

	static char buf[512];
	_sd.ReadSector(1, buf);

	abort();
}
