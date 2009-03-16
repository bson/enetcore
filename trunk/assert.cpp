#include "enetcore.h"
#include "trace.h"


void NORETURN AssertFailed(const char* expr, const char* file, int linenum)
{
	console("Assert failed in %s line %u:", file, linenum);
	console("Assert:   %s", expr);

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

	WaitForDebugger();
}
#else
void NORETURN PanicStop(const uchar* msg)
{
	console("Panic: %s", msg ? msg : STR(""));
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
