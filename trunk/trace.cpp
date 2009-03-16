#include "enetkit.h"
#include "serial.h"


// Console message

void console(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	Vector<uchar> msg(256);
//	const Time now = Time::Now();
#ifdef DEBUG
//	Util::AppendFmt(msg, STR("%t "), &now);
#else
//	Util::AppendFmt(msg, STR("%T "), &now);
#endif
	Util::AppendVFmt(msg, (const uchar*)fmt, va);
	va_end(va);
	msg.PushBack((uchar)'\r');
	msg.PushBack((uchar)'\n');

	_console.WriteSync(msg);
}


#if defined(TRACE)
void DMSG(const char* fmt, ...)
{
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

	_console.WriteSync(msg);
}
#endif
