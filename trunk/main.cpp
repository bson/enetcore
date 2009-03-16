#include "enetkit.h"
#include "serial.h"

int	main ()
{
	_malloc_region.SetReserve(4096);

	_lcd.WriteSync(String(STR("\xfe\1Enetcore 0.1 DEV")));
	_console.WriteSync(String(STR("Enetcore 0.1 DEV\r\n")));

	void* tmp = xmalloc(31);

	fault(3);
}
