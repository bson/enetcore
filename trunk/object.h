#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "freelist.h"


class Object {
public:
	enum Type {
		NULLTYPE = 0,
		STRING,
		NUMBER,
		DICT,
		ARRAY,
		TRUETYPE,
		FALSETYPE,
		// Not an actual type - used as wildcard for Retrieve()
		ANY_TYPE
	};

protected:
	// This is protected - create objects through Create()
	Object() { }
public:
	virtual ~Object() { }

	virtual Type GetType() const = 0;
	virtual void AsJson(Vector<uchar>& dest) = 0;
	virtual void AsXmlRpc(Vector<uchar>& dest) = 0;

	virtual void Delete() = 0;
	virtual Object* Clone() = 0;

	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);

	// XML-RPC parser - recognizes <methodCall> and ignores <? ... ?>
	static bool ParseXmlRpc(Deque<uchar>& buffer, Object*& ob);

	// Parses an XML-RPC call according to the format:
	//
	// <methodCall>
	//  <methodName>Echo</methodName>
	//  <params>
	//    <param>
	//       <struct>
	//          <member>
	//             <name>arg</name>
	//             <value><string>hello, world!</string></value>
	//          </member>
	//       </struct>
	//    </param>
	//    <param>
	//       <i4>123</i4>
	//    </param>
	//  </params>
	// </methodCall> 
	//
	// This will come out represented as:
	//
	// { "method": "Echo",
	//   "id": 123,
	//   "params": { "arg": "hello, world!" }
	// }
	//
	// I.e., the first positional parameter is a struct containing named params.
	// The second parameter is an integer, which is the call id.
	//
	// If there is no 'id' (second positional parameter) in the call, one is
	// made up.
	//
	// If the call doesn't match this layout the parameters are encoded as an
	// array. So if the second param in the example above were the string "xyz"
	// rather than the integer 123, or the first parameter were something other
	// than a struct, or there were more than two positional parameters, the call
	// would end up being represented as:
	//
	// { "method": "Echo",
	//   "params": [ { "arg": "hello, world!" }, "xyz" ]
	// }
	//
	// XML-RPC is indeed unbelievably verbose.
	//
	// NB: there is no way to represent null in XML-RPC.  It won't make it intact
	// across a transition from JSON-RPC to XML-RPC and back but will turn into
	// the string "null".  Oh well, don't use it.

	static bool ParseXmlRpcCall(Deque<uchar>& buffer, Object*& ob);
	static bool ParseXmlRpcParams(Deque<uchar>& buffer, Object*& ob);

	// Generate XML-RPC result with this Object as its value according to the format:
	// <?xml version="1.0"?>
	// <methodResponse>
	//    <params>
	//       <param>
	//          <value><string>South Dakota</string></value>
	//          </param>
	//       </params>
	//    </methodResponse>
	void AsXmlRpcResultResponse(Vector<uchar>& buffer);

	// Generate XML-RPC result with this Object as its central struct according to
	// the format below.  Note that this could be in DictObject, but we put it on
	// the Object base for logical grouping.
	//
	// <?xml version="1.0"?>
	// <methodResponse>
	//    <fault>
	//       <value>
	//          <struct>
	//             <member>
	//                <name>faultCode</name>
	//                <value><int>4</int></value>
	//                </member>
	//             <member>
	//                <name>faultString</name>
	//                <value><string>Too many parameters.</string></value>
	//                </member>
	//             </struct>
	//          </value>
	//       </fault>
	//    </methodResponse>
	void AsXmlRpcErrorResponse(Vector<uchar>& buffer);

	// Rewrite XML-RPC using the above convention of JSON-RPC 1.1 equivalence
	static void ToJsonEquivalence(Object*& ob);
	bool MethodCallAsXmlRpc(Vector<uchar>& buffer);

	static String ParseTag(Deque<uchar>& buffer);

	class DictObject* ToDict() { assert(GetType() == DICT); return (class DictObject*)this; }
	class ArrayObject* ToArray() { assert(GetType() == ARRAY); return (class ArrayObject*)this; }
	class StringObject* ToString() { assert(GetType() == STRING); return (class StringObject*)this; }
	class NumberObject* ToNumber() { assert(GetType() == NUMBER); return (class NumberObject*)this; }

	// Simplified object retrieval
	// First parameter is dot-notation dict node, e.g. "params.type"
	// Second parameter is expected type, or ANY_TYPE
	// Div is divider and can be changed if name needs to contain e.g. "."
	// Returns NULL if no object of desired type was found, otherwise returns object
	// Note: returns pointer into object tree
	Object* Retrieve(const String& name, Type expected_type = ANY_TYPE,
					 const String& div = STR("."));
};


class NullObject: public Object {
public:
	static NullObject* Alloc() { static NullObject n; return &n; }
	void Delete() { }

	Type GetType() const { return Object::NULLTYPE; }
	void AsJson(Vector<uchar>& dest) { dest.PushBack(STR("null")); }
	void AsXmlRpc(Vector<uchar>& dest) { dest.PushBack(STR("<nil/>")); }
	Object* Clone() { return this; }
	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);
};


class TrueObject: public Object {
public:
	static TrueObject* Alloc() { static TrueObject t; return &t; }
	void Delete() { }

	Type GetType() const { return Object::TRUETYPE; }
	void AsJson(Vector<uchar>& dest) { dest.PushBack(STR("true")); }
	void AsXmlRpc(Vector<uchar>& dest) { dest.PushBack(STR("<boolean>1</boolean>")); }
	Object* Clone() { return const_cast<TrueObject*>(this); }
	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);
	static bool ParseXmlRpc(Deque<uchar>& buffer, Object*& ob);
};


class FalseObject: public Object {
public:
	static FalseObject* Alloc() { static FalseObject f; return &f; }
	void Delete() { }

	Type GetType() const { return Object::FALSETYPE; }
	void AsJson(Vector<uchar>& dest) { dest.PushBack(STR("false")); }
	void AsXmlRpc(Vector<uchar>& dest) { dest.PushBack(STR("<boolean>0</boolean>")); }
	Object* Clone() { return const_cast<FalseObject*>(this); }
	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);
};


class StringObject: public Object {
	static Freelist<StringObject> _f;
	String _v;
protected:
	friend class Freelist<StringObject>;
	StringObject() { }
	StringObject(const String& arg) : _v(arg) { }
	virtual ~StringObject() { }

public:
	static StringObject* Alloc() { return _f.Alloc(); }
	static StringObject* Alloc(const String& arg) { return _f.Alloc(arg); }
	virtual void Delete() { _f.Free(this); }

	Type GetType() const { return Object::STRING; }
	void AsJson(Vector<uchar>& dest);
	void AsXmlRpc(Vector<uchar>& dest);
	virtual Object* Clone() { return Alloc(String(_v)); }
	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);
	static bool ParseXmlRpc(Deque<uchar>& buffer, Object*& ob, const String& endtag);

	StringObject& operator=(const String& arg) { _v = arg; return *this; }
//	const String GetString() const;
	const String& GetString() const { return _v; }

	static bool Order(const void* a, const void* b) {
		assert((*(Object**)a)->GetType() == Object::STRING);
		assert((*(Object**)b)->GetType() == Object::STRING);

		return (*(StringObject**)a)->GetString() > (*(StringObject**)b)->GetString();
	}
};


// Singleton version
class StaticStringObject: public StringObject {
private:
	StaticStringObject();
public:
	StaticStringObject(const String& arg) : StringObject(arg) { }

	void Delete() { }

	Object* Clone() { return const_cast<StaticStringObject*>(this); }
};

#define STROB(S) StaticStringObject(S)
	

class NumberObject: public Object {
	static Freelist<NumberObject> _f;
	// Note: JSON technically is floating point, but we only deal with integers
	int64_t _v;

protected:
	friend class StaticNumberObject;
	friend class Freelist<NumberObject>;
	NumberObject(int64_t arg = 0) : _v(arg) { }

public:
	static NumberObject* Alloc(int64_t arg = 0);
	virtual void Delete() { _f.Free(this); }

	Type GetType() const { return Object::NUMBER; }
	void AsJson(Vector<uchar>& dest);
	void AsXmlRpc(Vector<uchar>& dest);
	virtual Object* Clone() { return _f.Alloc(_v); }
	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);
	static bool ParseXmlRpc(Deque<uchar>& buffer, Object*& ob, const String& endtag);

	NumberObject& operator=(int64_t v) { _v = v; return *this; }
	int64_t GetInt() const { return _v; }
};

	
class StaticNumberObject: public NumberObject {
private:
	StaticNumberObject() { }
public:
	StaticNumberObject(int64_t arg = 0) : NumberObject(arg) { }

	void Delete() { }

	Object* Clone() { return const_cast<StaticNumberObject*>(this); }
};


inline NumberObject* NumberObject::Alloc(int64_t arg) {
	if (arg == 0) {
		static StaticNumberObject zero(0); return &zero;
	} else if (arg == 1) {
		static StaticNumberObject one(1); return &one;
	}

	return _f.Alloc(arg);
}


class DictObject: public Object {
	static Freelist<DictObject> _f;
public:
	typedef KeyValue<Object*, Object*, StringObject::Order> KV;
protected:
	friend class Freelist<DictObject>;
	Set<KV*, KV::Order> _v;
private:
	static Freelist<KV> _kvf;
public:
	DictObject() { }
	~DictObject();
	static DictObject* Alloc() { return _f.Alloc(); }
	void Delete() { _f.Free(this); }

	Type GetType() const { return Object::DICT; }
	void AsJson(Vector<uchar>& dest);
	void AsXmlRpc(Vector<uchar>& dest);
	Object* Clone();
	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);
	static bool ParseXmlRpc(Deque<uchar>& buffer, Object*& ob);

	void Insert(Object* key, Object* value);
	void Insert(const uchar* key, Object* value);
	void Erase(uint pos);
	void Erase(Object* key);
	void Erase(const uchar* key);
	KV*& operator[](uint pos) { return _v[pos]; }

	Object*& GetValue(const String& key);
};


class ArrayObject: public Object {
	static Freelist<ArrayObject> _f;
	Vector<Object*> _v;
public:
	ArrayObject() { }
	ArrayObject(uint n) : _v(n) { }
	~ArrayObject();
	static ArrayObject* Alloc(uint n = 0) { return n ? _f.Alloc(n) : _f.Alloc(); }
	void Delete() { _f.Free(this); }

	Type GetType() const { return Object::ARRAY; }
	void AsJson(Vector<uchar>& dest);
	void AsXmlRpc(Vector<uchar>& dest);
	Object* Clone();
	static bool ParseJson(Deque<uchar>& buffer, Object*& ob);
	static bool ParseXmlRpc(Deque<uchar>& buffer, Object*& ob);

	void PushBack(Object* value) { _v.PushBack(value); }
	Object*& operator[](uint pos) { return _v[pos]; }
	const Object* operator[](uint pos) const { return _v[pos]; }
	uint Size() const { return _v.Size(); }
};


#endif // __OBJECT_H__
