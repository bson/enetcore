// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#ifndef __PSTRING_H__
#define __PSTRING_H__

#include "lookup3.h"

#pragma GCC system_header


// Dumb 0 terminated C string
// POD safe

class String {

	// XXX there is currenly a bug with freelisting strings
	// HashMap<> will try to delete a String*, which will mess up the freelist.
#ifdef STRING_FREELIST
	static Freelist<String> _f;
#endif
	uchar* _v;

public:
	String() : _v(xstrdup(STR(""))) { }
	String(const uchar* s) {
		assert(s);
		_v = xstrdup(s);
	}
	String(const char* s) {
		assert(s);
		_v = xstrdup(STR(s));
	}
	String(const uchar* s, uint n) {
        assert(s);
        _v = xmemtostr(s, n);
        assert(!IsLiteral(_v));
    }
	String(const String& s) {
		assert(s._v);
		_v = xstrdup(s._v);
	}
	String(const Vector<uchar>& v) {
		_v = (uchar*)xmalloc(v.Size() + 1);
		if (v.Size())
			::memcpy(_v, v + 0, v.Size());

		_v[v.Size()] = 0;
	}

	~String() { xfree(_v); }

	String& assign(const String& s) {
		if (&s != this) {
			if (!IsLiteral(_v) && Size() >= s.Size()) {
				assert(s._v);
				xstrcpy(_v, s._v);
			} else {
				xfree(_v);
				assert(s._v);
				_v = xstrdup(s._v);
			}
		}
		return *this;
	}

#ifdef STRING_FREELIST
	static void CheckFreeList(void* ptr) { _f.Check(ptr); }
	static String* Alloc() { return _f.Alloc(); }
	static String* Alloc(const String& arg) { return ::new (_f.AllocMem()) String(arg); }
	static String* Alloc(const uchar* s) { return ::new (_f.AllocMem()) String(s); }
	static String* Alloc(const char* s) { return ::new (_f.AllocMem()) String(s); }
	void Delete() { _f.Free(this);  }
#else
	static void CheckFreeList(void*) { }
	static String* Alloc() { return ::new String(); }
	static String* Alloc(const String& arg) { return ::new String(arg); }
	static String* Alloc(const uchar* s) { return ::new String(s); }
	static String* Alloc(const char* s) { return ::new String(s); }
	void Delete() { delete this;  }
#endif

	String& operator=(const String& arg) { return assign(arg); }

	uchar* Grab(uint& used, uint& start, uint& alloc) {
		used = Size();
		alloc = used + 1;
		start = 0;
		return exch<uchar*>(_v, xstrdup(STR("")));
	}

	template <typename TT> String& Take(TT& arg) {
		xfree(_v);
		arg.PushBack((uchar)0);
		uint tmp1, tmp2, tmp3;
		_v = arg.Grab(tmp1, tmp2, tmp3);
		return *this;
	}

	static String Format(const uchar* fmt, ...);
	static String VFormat(const uchar* fmt, va_list& va);

	// This is needed for String to Take a String
	void PushBack(uchar c) { assert(!c); }

	uint Size() const { assert(_v); return ::xstrlen(_v); }
	const uchar* CStr() const { return _v; }
	bool Empty() const { assert(_v); return !_v[0]; }
	void Clear() { xfree(_v); _v = xstrdup(STR("")); }
	
	String Substr(uint from, uint len) const { assert(_v); return String(_v + from, len); }
	String Head(uint len) const { assert(_v); return String(_v, len); }
	String Tail(uint pos) const { assert(_v); return _v + pos; }
	String Trim(uchar what) const {
		assert(_v);
		for (uint pos = 0; _v[pos]; ++pos)
			if (_v[pos] != what)  return _v + pos;
		return STR("");
	}

	bool BeginsWith(const String& arg) const {
		const uint arglen = arg.Size();
		return arglen <= Size() && xstrncasecmp(arg._v, _v, arglen) == 0;
	}

	bool EndsWith(const String& arg) const {
		const uint arglen = arg.Size();
		const uint len = Size();
		return arglen <= len && xstrncasecmp(_v + len - arglen, arg._v, arglen) == 0;
	}

	uint FindFirst(uchar c) const {
		const uchar* s = xstrchr(_v, c);
		if (!s) return NOT_FOUND;

		return s - _v;
	}

	// Takes ownership of arg
	String& TakeCStr(uchar* arg) { xfree(_v); _v = arg; return *this; }

	uint FindFirst(const String& s) const {
		assert(_v);
		assert(s._v);
		const uchar* p = xstrstr(_v, s._v);
		if (!p) return NOT_FOUND;

		return p - _v;
	}

	uint FindLast(uchar c) const {
		assert(_v);
		const uchar* s = xstrrchr(_v, c);
		if (!s) return NOT_FOUND;

		return s - _v;
	}

	uchar& operator[](uint pos) { assert(_v); return _v[pos]; }
	uchar operator[](uint pos) const { assert(_v); return _v[pos]; }

	bool Equals(const String& arg, bool case_sensitive = false) const {
		return case_sensitive ? !xstrcmp(_v, arg._v) : !xstrcasecmp(_v, arg._v);
	}

	bool operator==(const String& arg) const { assert(_v); return xstrcmp(_v, arg._v) == 0; }
	bool operator!=(const String& arg) const { assert(_v); return xstrcmp(_v, arg._v) != 0; }
	bool operator>(const String& arg) const { assert(_v); return xstrcmp(_v, arg._v) > 0; }
	bool operator<(const String& arg) const { assert(_v); return xstrcmp(_v, arg._v) < 0; }
	bool operator>=(const String& arg) const { assert(_v); return xstrcmp(_v, arg._v) >= 0; }
	bool operator<=(const String& arg) const { assert(_v); return xstrcmp(_v, arg._v) <= 0; }

	void Split(Vector<String*>& list, const String& divider, uint maxitems = (uint)-1) const;

	String UrlDecode() const;
	String UrlEncode() const;

	// Append
	String& operator+=(const String& arg);

	// For OContainer<String>
	static bool Order(const void* a, const void* b);

	// For HashTable<>
	static void Hash(const void* vs, uint32_t& hash1, uint32_t& hash2) { 
		const String& s = *(String*)vs;
		assert(s._v);
		Lookup3::hashlittle2(s._v, s.Size(), &hash1, &hash2);
	}

	static bool HashEqual(const void* a, const void* b) {
		return *(String*)a == *(String*)b;
	}
};

#endif // __PSTRING_H__
