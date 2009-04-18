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

	File* file = _fat.Open("test.txt");

	if (file) {
		uint8_t* buffer = (uint8_t*)xmalloc(100);

		uint len = file->Read(buffer, 99);
		buffer[len] = 0;
		
		DMSG("test.txt: \"%s\"", buffer);
		xfree(buffer);

	}

	abort();
}
