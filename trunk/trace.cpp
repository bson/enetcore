#include "enetkit.h"
#include "serial.h"


// Console message

void console(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
#if 1
	const Time now = Time::Now();
	_console.Write(String::Format(STR("%t "), &now));
	_console.Write(String::VFormat((const uchar*)fmt, va));
	_console.Write(STR("\r\n"));
	va_end(va);
#else
	Vector<uchar> msg(256);
	const Time now = Time::Now();
#ifdef DEBUG
	Util::AppendFmt(msg, STR("%t "), &now);
#else
	Util::AppendFmt(msg, STR("%T "), &now);
#endif
	Util::AppendVFmt(msg, (const uchar*)fmt, va);
	msg.PushBack((uchar)'\r');
	msg.PushBack((uchar)'\n');
	_console.Write(msg);
	va_end(va);
#endif
}


#if defined(TRACE)
void DMSG(const char* fmt, ...)
{
#if 1
	va_list va;
	va_start(va, fmt);
	const Time now = Time::Now();
	_console.Write(String::Format(STR("%t DEBUG "), &now));
	_console.Write(String::VFormat((const uchar*)fmt, va));
	_console.Write(STR("\r\n"));
	va_end(va);
#else
	Vector<uchar> msg(256);
//	const Time now = Time::Now();
#ifdef DEBUG
//	Util::AppendFmt(msg, STR("%t "), &now);
#else
//	Util::AppendFmt(msg, STR("%T "), &now);
#endif
	msg.PushBack((const uchar*)"DEBUG: ");
	va_list va;
	va_start(va, fmt);
	Util::AppendVFmt(msg, (const uchar*)fmt, va);
	va_end(va);
	msg.PushBack((uchar)'\r');
	msg.PushBack((uchar)'\n');

	_console.Write(msg);
#endif
}
#endif
