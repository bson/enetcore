#include "enetkit.h"
#include "trace.h"
#include "serial.h"


void NORETURN AssertFailed(const char* expr, const char* file, int linenum)
{
	console("Assert failed in %s line %u:", file, linenum);
	console("Assert:   %s", expr);

	const String lcdmsg = String::Format(STR("\xfe\001@%s:%u\xfe\xc0%s"), file, linenum, expr);
	_lcd.WriteSync(lcdmsg);

#ifdef DEBUG
	WaitForDebugger();
#else
	fault(4);
#endif
	for (;;) ;
}


#ifdef DEBUG
void NORETURN PanicStop(const uchar* msg, const char* file, int linenum)
{
	console("Panic: %s line %u:", file, linenum);
	console("Panic:    %s", msg ? msg : STR("No panic string"));

	const String lcdmsg = String::Format(STR("\xfe\001Panic: %s\xfe\xc0" "%s:%u"), msg, file, linenum);
	_lcd.WriteSync(lcdmsg);

	WaitForDebugger();
}
#else
void NORETURN PanicStop(const uchar* msg)
{
	console("Panic: %s", msg ? msg : STR(""));

	const String lcdmsg = String::Format(STR("\xfe\001Panic: %s"), msg);
	_lcd.WriteSync(lcdmsg);

	fault(4);
	for (;;) ;
}
#endif

#ifdef DEBUG
void NORETURN WaitForDebugger()
{

	fault(4);
	for (;;) ;					// g++ is pretty stupid
}
#endif
