#include "enetkit.h"
#include "util.h"
#include "sha1.h"
//#include "object.h"
#include "mutex.h"


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



void FormatNumber(Vector<uchar>& dest, uint64_t val, uint flags, uint radix, uint digits)
{
	assert(radix <= 16);
	assert(digits <= 31);

	if (!(flags & FMT_UNSIGNED) && val < 0) {
		dest.PushBack((uchar)'-');
		val = -val;
	}

	const uchar pad = flags & FMT_SPACEPAD ? ' ' : '0';

	uchar buf[32];
	buf[31] = 0;
	uchar* pos = buf + 31;

	if (val) {
		while (val && pos > buf) {
			const uint digit = val % radix;
			assert(digit <= 15);
			if (digit <= 9)  *--pos = digit + '0';
			else *--pos = (uchar)(digit - 10 + 'a');
			val /= radix;
			if (digits) --digits;
		}
	} else {
		*--pos = '0';
		if (digits) --digits;
	}

	while (digits--)  *--pos = pad;

	dest.PushBack(pos);
}


void FormatTime(Vector<uchar>& dest, const Time* t, bool with_date)
{
	assert(t);
	
#ifdef POSIX
	static Mutex lock;
	Mutex::Scoped L(lock);

	const time_t posixtime = t->GetPosixTime();
	struct tm* tm = ::localtime(&posixtime);
#else
	struct tm tm0;
	t->ToCalendar(tm0);
	const tm* const tm = &tm0;

	static Mutex lock;
	Mutex::Scoped L(lock);
#endif
	if (with_date)
		AppendFmt(dest, STR("%04u-%02u-%02u %02u:%02u:%02u"),
				  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
				  tm->tm_hour, tm->tm_min, tm->tm_sec);
	else
		AppendFmt(dest, STR("%02u:%02u:%02u.%03u"),
				   tm->tm_hour, tm->tm_min, tm->tm_sec, t->GetMsec() % 1000);
}


void FormatCString(Vector<uchar>& dest, const uchar* s, uint flags, uint param)
{
	uint len = xstrlen(s);
	if (param)  {
		// Truncate string at specified length
		len = min(len, param);

		// Calculate pad needed
		param -= min(param, len);
	}

	if (!(flags & FMT_LEFT)) {
		while (param) {
			dest.PushBack((uchar)' ');
			--param;
		}
	}
	dest.PushBack(s, len);

	while (param--) dest.PushBack((uchar)' ');
}


const uint AppendVFmt(Vector<uchar>& dest, const uchar* fmt, va_list& va)
{
	const uint result = dest.Size();

	while (*fmt) {
		if (*fmt == '%') {
			int flags = 0;
			int param = 0;
			if (fmt[1] == ':') {
				flags |= FMT_COLON;
				++fmt;
			}
			if (fmt[1] == '.') {
				flags |= FMT_DOT;
				++fmt;
			}
			if (fmt[1] == '-') {
				flags |= FMT_LEFT | FMT_SPACEPAD;
				++fmt;
			} else if (fmt[1] == '0') {
				flags |= FMT_ZEROPAD;
				++fmt;
			}

			while (fmt[1] >= '0' && fmt[1] <= '9') {
				param *= 10;
				param += fmt[1] - '0';
				++fmt;
			}

			switch (*++fmt) {
			case '%':  dest.PushBack('%'); break;
			case 'd':  {
				FormatNumber(dest, va_arg(va, int), flags, 10, param);
				break;
			}
			case 'D':  {
				FormatNumber(dest, va_arg(va, int64_t), flags, 10, param);
				break;
			}
			case 'u': {
				FormatNumber(dest, va_arg(va, uint), flags | FMT_UNSIGNED, 10, param);
				break;
			}
			case 'U': {
				FormatNumber(dest, va_arg(va, uint64_t), flags | FMT_UNSIGNED, 10, param);
				break;
			}
			case 'x': {
				FormatNumber(dest, va_arg(va, uint), flags | FMT_UNSIGNED, 16, param);
				break;
			}
			case 'X': {
				FormatNumber(dest, va_arg(va, uint64_t), flags | FMT_UNSIGNED, 16, param);
				break;
			}
			case 'p': {
				dest.PushBack(STR("0x"));
				FormatNumber(dest, va_arg(va, uintptr_t), flags | FMT_UNSIGNED, 16, sizeof(uintptr_t)*2);
				break;
			}
			case 'c': {
				dest.PushBack(va_arg(va, int));
				break;
			}
			case 's': {
				FormatCString(dest, va_arg(va, const uchar*), flags, param);
				break;
			}
			case 'S': {
				FormatCString(dest, va_arg(va, const String*)->CStr(), flags, param);
				break;
			}
			case 'a':
			case 'A': {
				dest.PushBack(va_arg(va, const NetAddr*)->Printable(*fmt == 'A').CStr());
				break;
			}
#if 0
			case 'o': {
				Object* o = va_arg(va, Object*);
				if (o) o->AsJson(dest);
				else dest.PushBack((const uchar*)"(null)");
				break;
			}
#endif
			case 'h': {
				uint len = param ? param : 20;
				for (const uchar* p = va_arg(va, uchar*); len > 0; --len, ++p) {
					FormatNumber(dest, *p, FMT_UNSIGNED, 16, 2);
					if ((flags & FMT_COLON) && len > 1)
						dest.PushBack((uchar)':');
				}

				break;
			}

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
