// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"


[[noreturn]] void AssertFailed(const char* expr, const char* file, int linenum) {
    DisableInterrupts();

    while (_assert_stop)
        ;

    // In case it faults again
    _assert_stop = true;

	_console.SyncDrain();

    if (InExceptionHandler()) {
        // formatting uses memory allocation
        _console.WriteCStr("\r\nAssert failed in ");
        _console.WriteCStr(file);
        _console.WriteCStr("\r\nAssert:   ");
        _console.WriteCStr(expr);
        _console.WriteCStr("\r\n");
    } else {
        console("Assert failed in %s line %u:", file, linenum);
        console("assert:    %s", (const uchar*)expr ? (const uchar*)expr :
                STR("<no expression>"));
    }

	_console.SyncDrain();

#ifdef DEBUG
	WaitForDebugger();
#else
	fault(4);
#endif
	for (;;) ;
}


#ifdef DEBUG
[[noreturn]] void PanicStop(const uchar* msg, const char* file, int linenum) {
    DisableInterrupts();

    while (_assert_stop)
        ;

    // In case it faults again
    _assert_stop = true;

	_console.SyncDrain();

    if (InExceptionHandler()) {
        // formatting uses memory allocation
        _console.WriteCStr("\r\nPanic in file ");
        _console.WriteCStr(file);
        _console.WriteCStr("\r\nPanic:   ");
        _console.WriteCStr((const char*)msg);
        _console.WriteCStr("\r\n");
    } else {
        console("Panic: %s line %u:", file, linenum);
        console("Panic:    %s", msg ? msg : STR("No panic string"));
    }

	_console.SyncDrain();

	WaitForDebugger();
}
#else
[[noreturn]] void PanicStop(const uchar* msg)
{
    DisableInterrupts();

    while (_assert_stop)
        ;

    // In case it faults again
    _assert_stop = true;

    if (InExceptionHandler()) {
        // formatting uses memory allocation
        _console.WriteCStr("\r\nPanic:   ");
        _console.WriteCStr((const char*)msg);
        _console.WriteCStr("\r\n");
    } else {
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
