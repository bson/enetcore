#include "enetkit.h"


// Console message

void console(const char* fmt, ...)
{
#if 0
	va_list va;
	va_start(va, fmt);
	Vector<uchar> msg(256);
	const Time now = Time::Now();
#ifdef DEBUG
	Util::AppendFmt(msg, STR("%t "), &now);
#else
	Util::AppendFmt(msg, STR("%T "), &now);
#endif
	Util::AppendVFmt(msg, (const uchar*)fmt, va);
	va_end(va);
	msg.PushBack((uchar)'\n');

	::write(1, msg + 0, msg.Size());
#endif
}


#if defined(TRACE)
void DMSG(const char* fmt, ...)
{
#if 0
	Vector<uchar> msg(256);
	const Time now = Time::Now();
#ifdef DEBUG
	Util::AppendFmt(msg, STR("%t "), &now);
#else
	Util::AppendFmt(msg, STR("%T "), &now);
#endif
	msg.PushBack((const uchar*)"DEBUG: ");
	va_list va;
	va_start(va, fmt);
	Util::AppendVFmt(msg, (const uchar*)fmt, va);
	va_end(va);
	msg.PushBack((uchar)'\n');

	::write(1, msg + 0, msg.Size());
#endif
}
#endif
