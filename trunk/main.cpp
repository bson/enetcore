#include "enetkit.h"
#include "serial.h"

#define SERLCD_CMD "\xfe"
#define SERLCD_CLEAR SERLCD_CMD "\x01"
#define SERLCD_BS SERLCD_CMD "\x10"
#define SERLCD_SETCUR SERLCD_CMD "\x80"

int	main ()
{
	hwinit();

	_lcd.WriteSync(STR("  \xfe\x01Enetcore v0 DEV"), 19);
	_console.WriteSync(STR("Enetcore v0 DEV\r\n"), 17);
	_console.WriteSync(String(STR("This is a test\r\n")));

	fault(3);
}
