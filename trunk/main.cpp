#include "enetkit.h"
#include "serial.h"


int	main ()
{
	hwinit();

	_lcd.WriteSync(STR("Enetcore v0 DEV"), 15);
	_console.WriteSync(STR("Enetcore v0 DEV\r\n"), 17);

	fault(3);
}
