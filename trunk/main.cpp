#include "enetkit.h"
#include "serial.h"
#include "util.h"
#include "sdcard.h"
#include "fat.h"


Fat _fat(_sd);

int	main()
{
	_malloc_region.SetReserve(4096);

	_lcd.Write(String(STR("\xfe\1Enetcore 0.1 DEV")));
	_lcd.SyncDrain();

	_fat.Mount(0, false);

	file_t file = _fat.Open("test.txt");

	abort();
}
