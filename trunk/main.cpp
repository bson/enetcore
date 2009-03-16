#include "enetkit.h"
#include "serial.h"

#define SERLCD_CMD "\xfe"
#define SERLCD_CLEAR SERLCD_CMD "\x01"
#define SERLCD_BS SERLCD_CMD "\x10"
#define SERLCD_SETCUR SERLCD_CMD "\x80"

int	main ()
{
	_lcd.WriteSync(STR("\254\1" "Enetcore v0 DEV"), 19);
	_console.WriteSync(String(STR("Enetcore v0 DEV\r\n")));

	fault(3);
}
