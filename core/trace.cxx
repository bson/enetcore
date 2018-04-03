// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"


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
	_console.Write(String::VFormat((const uchar*)fmt, va));
	_console.Write(STR("\r\n"));
	va_end(va);
}
#endif
