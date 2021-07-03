// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"


[[noreturn]] void AssertFailed(const char* expr, const char* file, int linenum) {
    while (_assert_stop)
        ;

    // In case it faults again
    _assert_stop = true;

    if (!InExceptionHandler()) {
        console("Assert failed in %s line %u:", file, linenum);
        console("assert:    %s", (const uchar*)expr ? (const uchar*)expr :
                STR("<no expression>"));
    }

	_console.SyncDrain();

    DisableInterrupts();
#ifdef DEBUG
	WaitForDebugger();
#else
	fault(4);
#endif
	for (;;) ;
}


#ifdef DEBUG
[[noreturn]] void PanicStop(const uchar* msg, const char* file, int linenum) {
    while (_assert_stop)
        ;

    // In case it faults again
    _assert_stop = true;

    if (!InExceptionHandler()) {
        console("Panic: %s line %u:", file, linenum);
        console("Panic:    %s", msg ? msg : STR("No panic string"));
    }

	_console.SyncDrain();

	WaitForDebugger();
}
#else
[[noreturn]] void PanicStop(const uchar* msg)
{
    while (_assert_stop)
        ;

    // In case it faults again
    _assert_stop = true;

    if (!InExceptionHandler()) {
        console("Panic:    %s", msg ? msg : STR(""));
    }

	_console.SyncDrain();

    // XXX should reset here
	fault(4);
    for (;;);
}
#endif
bool _assert_stop;

#ifdef DEBUG
[[noreturn]] void WaitForDebugger()
{
    // Do nothing if not configured yet
    fault(4);
	for (;;) ;
}
#endif
