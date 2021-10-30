// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#include "core/enetkit.h"
#include "core/util.h"


#ifdef STRING_FREELIST
Freelist<String> String::_f;
#endif

void String::Split(Vector<String*>& list, const String& divider, uint maxitems) const
{
	uint pos = 0;
	const uint divlen = divider.Size();
	const uchar* next;

	assert(maxitems);

	while ((next = xstrstr(_v + pos, divider._v)) && --maxitems) {
#ifdef STRING_FREELIST
		list.PushBack(::new (_f.AllocMem()) String(_v + pos, next - _v - pos));
#else
		list.PushBack(::new String(_v + pos, next - _v - pos));
#endif
		Platform::_malloc_region.Validate(list.Back());

		pos = next - _v + divlen;
	}

#ifdef STRING_FREELIST
	list.PushBack(::new (_f.AllocMem()) String(_v + pos, xstrlen(_v)));
#else
 	list.PushBack(::new String(_v + pos, xstrlen(_v + pos)));
#endif
	Platform::_malloc_region.Validate(list.Back());
}


// * static
bool String::Order(const void* a, const void* b)
{
	const String& s1 = **(String**)a;
	const String& s2 = **(String**)b;

	return s1 > s2;
}


// * static
String String::Format(const uchar* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	Vector<uchar> v(64);
	Util::AppendVFmt(v, fmt, va);
	va_end(va);

	return String().Take(v);
}


// * static
String String::VFormat(const uchar* fmt, va_list& va)
{
	Vector<uchar> v(64);
	Util::AppendVFmt(v, fmt, va);
	return String().Take(v);
}


String String::UrlDecode() const
{
	Vector<uchar> result(64);

	const uint arglen = Size();
	for (uint i = 0; i < arglen; ++i) {
		// Can't start %xx with less than three uchars left to go
		if (i >= arglen - 2 || _v[i] != '%')  {
			result.PushBack(_v[i]);
			continue;
		}

		const int d1 = Util::ParseHexDigit(_v[i+1]);
		const int d2 = Util::ParseHexDigit(_v[i+2]);
		if (d1 != -1 && d2 != -1) {
			assert(d1 >= 0);
			assert(d1 <= 15);
			assert(d2 >= 0);
			assert(d2 <= 15);
			result.PushBack(d1 * 16 + d2);
			i += 2;
		} else {
			result.PushBack('%');
		}
	}

	return String().Take(result);
}

	
String& String::operator+=(const String& arg)
{
	if (!arg.Empty()) {
		const uint len = Size();

		if (IsLiteral(_v)) {
			const uchar* tmp = _v;
			_v = (uchar*)xmalloc(len + arg.Size() + 1);
			::memcpy(_v, tmp, len);
		} else {
			_v = (uchar*)xrealloc(_v, len + arg.Size() + 1);
		}
		xstrcpy(_v + len, arg._v);
	}

	return *this;
}


String String::UrlEncode() const
{
	Vector<uchar> result(64);

	const uint arglen = Size();
	for (uint i = 0; i < arglen; ++i) {
		const uint8_t c = _v[i];
		if (c <= ' ' || c >= 127 || ::strchr("$&+,/:;=?@'\"<>#%{}|\\~`^[]", (char)c)) {
			result.PushBack('%');
			result.PushBack("0123456789ABCDEF"[c / 16]);
			result.PushBack("0123456789ABCDEF"[c & 15]);
		} else
			result.PushBack(c);
	}

	return String().Take(result);
}
