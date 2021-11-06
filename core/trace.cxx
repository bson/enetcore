// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetkit.h"


// Console message

void console(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	_console.Write(String::VFormat((const uchar*)fmt, va));
	_console.Write(STR("\r\n"));
	va_end(va);
}


#if defined(TRACE)
void DMSG(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	_trace.Apply(String::VFormat((const uchar*)fmt, va));
	_trace.Apply(STR("\r\n"));
	va_end(va);
}
#endif
