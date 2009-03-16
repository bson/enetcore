#include "enetkit.h"
#include "util.h"
//#include "netaddr.h"
#include "sha1.h"
#include "object.h"


namespace Util {

int ToUpper(int c)
{
	if (c >= 'a' && c <= 'z')  return c - 32;

	return c;
}


int ParseHexDigit(int c)
{
	if (c >= '0' && c <= '9')  return c - '0';

	c = ToUpper(c);
	if (c >= 'A' && c <= 'F')  return c - 'A' + 10;

	return -1;
}


void FormatTime(Vector<uchar>& dest, const Time* t, bool with_date)
{
	assert(t);
	const time_t posixtime = t->GetPosixTime();
	
	static Spinlock lock;
	Spinlock::Scoped L(lock);

#ifdef POSIX
	struct tm* tm = ::localtime(&posixtime);
	char buf[64];
	if (with_date)
		::snprintf(buf, sizeof buf, "%04u-%02u-%02u %02u:%02u:%02u",
				   tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
				   tm->tm_hour, tm->tm_min, tm->tm_sec);
	else
		::snprintf(buf, sizeof buf, "%02u:%02u:%02u.%03u",
				   tm->tm_hour, tm->tm_min, tm->tm_sec, t->GetMsec() % 1000);

	dest.PushBack((uint8_t*)buf);
#endif
}


const uint AppendVFmt(Vector<uchar>& dest, const uchar* fmt, va_list& va)
{
	const uint result = dest.Size();

	while (*fmt) {
		if (*fmt == '%') {
			switch (*++fmt) {
			case '%':  dest.PushBack('%'); break;
#if 0
			case 'd':  {
				// XXX stdio dependency
				uchar buf[16];
				::snprintf((char*)buf, sizeof buf, "%d", va_arg(va, int));
				dest.PushBack(buf);
				break;
			}
			case 'D':  {
				uchar buf[32];
				::snprintf((char*)buf, sizeof buf, "%lld", va_arg(va, int64_t));
				dest.PushBack(buf);
				break;
			}
			case 'u': {
				uchar buf[16];
				::snprintf((char*)buf, sizeof buf, "%u", va_arg(va, uint));
				dest.PushBack(buf);
				break;
			}
			case 'U': {
				uchar buf[32];
				::snprintf((char*)buf, sizeof buf, "%llu", va_arg(va, uint64_t));
				dest.PushBack(buf);
				break;
			}
			case 'x': {
				uchar buf[16];
				::snprintf((char*)buf, sizeof buf, "%x", va_arg(va, uint));
				dest.PushBack(buf);
				break;
			}
			case 'X': {
				uchar buf[32];
				::snprintf((char*)buf, sizeof buf, "%llx", va_arg(va, uint64_t));
				dest.PushBack(buf);
				break;
			}
			case 'p': {
				uchar buf[32];
				::snprintf((char*)buf, sizeof buf, "%p", va_arg(va, void*));
				dest.PushBack(buf);
				break;
			}
#endif
			case 'c': {
				dest.PushBack(va_arg(va, int));
				break;
			}
			case 's': {
				dest.PushBack(va_arg(va, const uchar*));
				break;
			}
			case 'S': {
				dest.PushBack(va_arg(va, const String*)->CStr());
				break;
			}
#if 0
			case 'a':
			case 'A': {
				dest.PushBack(va_arg(va, const NetAddr*)->Printable(*fmt == 'A').CStr());
				break;
			}
#endif
			case 'o': {
				Object* o = va_arg(va, Object*);
				if (o) o->AsJson(dest);
				else dest.PushBack((const uchar*)"(null)");
				break;
			}
#if 0
			case 'h': {
				uchar buf[5];
				uint len = 20;
				for (const uchar* p = va_arg(va, uchar*); len > 0; --len, ++p) {
					::snprintf((char*)buf, sizeof buf, "%02x", *p);
					dest.PushBack(buf);
				}
				break;
			}
#endif
			// yyyy-MM-dd hh:mm:ss
			case 'T': {
				FormatTime(dest, va_arg(va, Time*), true);
				break;
			}
			// hh:mm:ss.msec
			case 't': {
				FormatTime(dest, va_arg(va, Time*), false);
				break;
			}
			case 0:
				--fmt;
			default:
				dest.PushBack(*fmt);
				break;
			}
		} else {
			dest.PushBack(*fmt);
		}
		++fmt;
	}
	return result;
}


void Trim(Deque<uchar>& buffer)
{
	while (!buffer.Empty() && buffer[0] <= ' ')
		buffer.PopFront();
}


ComputerID::ComputerID()
{
	uint8_t mac[6+8];
	const bool ok = Platform::GetMacAddr(mac);
	if (!ok)  {
		console("Unable to obtain MAC address");
		::memcpy(mac, "abcdef", 6);
	}

	Sha1::Hash(mac, 6, id);

	// Initialize random sequence gen
	// Uses mac and 8-byte wall clock
	// Use digest so even if the seed is uncovered the mac addr is safe.
	const uint64_t now = Time::Now().GetUsec();
	::memcpy(mac + 6, &now, 8);
	uint8_t seed[20];
	Sha1::Hash(mac, sizeof mac, seed);

	Util::RandomSeed(seed, sizeof seed);
}


const ComputerID& GetComputerID()
{
	static ComputerID id;
	return id;
}


bool VoidStarOrder(const void* a, const void* b)
{
	return *(const void**)a > *(const void**)b;
}


} // namespace Util
