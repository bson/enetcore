#include "enetkit.h"
#include "trace.h"
#include "serial.h"


void __noreturn AssertFailed(const char* expr, const char* file, int linenum)
{
	console("Assert failed in %s line %u:", file, linenum);
	console("Assert:   %s", expr);

	const String lcdmsg = String::Format(STR("\xfe\001@%s:%u\xfe\xc0%s"), file, linenum, expr);
	_lcd.Write(lcdmsg);
	_lcd.SyncDrain();
	_console.SyncDrain();

#ifdef DEBUG
	WaitForDebugger();
#else
	fault(4);
#endif
	for (;;) ;
}


#ifdef DEBUG
void __noreturn PanicStop(const uchar* msg, const char* file, int linenum)
{
	console("Panic: %s line %u:", file, linenum);
	console("Panic:    %s", msg ? msg : STR("No panic string"));

	const String lcdmsg = String::Format(STR("\xfe\001Panic: %s\xfe\xc0" "%s:%u"), msg, file, linenum);
	_lcd.Write(lcdmsg);
	_lcd.SyncDrain();
	_console.SyncDrain();

	WaitForDebugger();
}
#else
void __noreturn PanicStop(const uchar* msg)
{
	console("Panic: %s", msg ? msg : STR(""));

	const String lcdmsg = String::Format(STR("\xfe\001Panic: %s"), msg);
	_lcd.Write(lcdmsg);
	_lcd.SyncDrain();
	_console.SyncDrain();

	fault(4);
	for (;;) ;
}
#endif

#ifdef DEBUG
void __noreturn WaitForDebugger()
{

	fault(4);
	for (;;) ;					// g++ is pretty stupid
}
#endif
