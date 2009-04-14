#include "enetkit.h"
#include "serial.h"
#include "util.h"
#include "sdcard.h"


int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.Write(String(STR("\xfe\1Enetcore 0.1 DEV")));
	_lcd.SyncDrain();

	_sd.Init();

	Deque<uint8_t> buf;
	_sd.ReadSector(0, buf);

	abort();
}
