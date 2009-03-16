#include "enetkit.h"
#include "object.h"
#include "util.h"
#include "freelist.h"


Freelist<StringObject> StringObject::_f;
Freelist<NumberObject> NumberObject::_f;
Freelist<DictObject> DictObject::_f;
Freelist<DictObject::KV> DictObject::_kvf;
Freelist<ArrayObject> ArrayObject::_f;


// * static
bool Object::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	Util::Trim(buffer);

	return NullObject::ParseJson(buffer, ob) ||
		TrueObject::ParseJson(buffer, ob) ||
		FalseObject::ParseJson(buffer, ob) ||
		StringObject::ParseJson(buffer, ob) ||
		DictObject::ParseJson(buffer, ob) ||
		ArrayObject::ParseJson(buffer, ob) ||
		NumberObject::ParseJson(buffer, ob);
}


String Object::ParseTag(Deque<uchar>& buffer)
{
	Util::Trim(buffer);
	
	if (buffer.Empty() || buffer[0] != '<')  return STR("");

	buffer.PopFront();

	Vector<uchar> result;
	while (!buffer.Empty() && buffer[0] != '>') {
		result.PushBack(buffer[0]);
		buffer.PopFront();
	}

	if (!buffer.Empty())  buffer.PopFront();

	return String().Take(result);
}


bool Object::ParseXmlRpc(Deque<uchar>& buffer, Object*& ob)
{
	for (;;) {
		const String tag = ParseTag(buffer);

		if (tag.Empty())
			return StringObject::ParseXmlRpc(buffer, ob, STR(""));

		if (tag[0] == '?') continue;

		// See http://ontosys.com/xml-rpc/extensions.php
		if (tag.Equals(STR("nil/"))) {
			ob = NullObject::Alloc();
			return true;
		}

		if (tag.Equals(STR("i4")))
			return NumberObject::ParseXmlRpc(buffer, ob, STR("/i4"));

		if (tag.Equals(STR("int")))
			return NumberObject::ParseXmlRpc(buffer, ob, STR("/int"));

		if (tag.Equals(STR("boolean")))
			return TrueObject::ParseXmlRpc(buffer, ob); // Parses both true and false

		if (tag.Equals(STR("struct")))
			return DictObject::ParseXmlRpc(buffer, ob);

		if (tag.Equals(STR("array")))
			return ArrayObject::ParseXmlRpc(buffer, ob);

		if (tag.Equals(STR("string")))
			return StringObject::ParseXmlRpc(buffer, ob, STR("/string"));

		if (tag.Equals(STR("methodCall")))
			return ParseXmlRpcCall(buffer, ob);

		return false;
	}
}


bool Object::ParseXmlRpcCall(Deque<uchar>& buffer, Object*& ob)
{
	ob = DictObject::Alloc();

	for (;;) {
		const String tag = ParseTag(buffer);

		if (tag.Equals(STR("/methodCall")))  {
			ToJsonEquivalence(ob);
			return true;
		}

		if (tag.Equals(STR("methodName"))) {
			Object* name = NULL;
			if (!StringObject::ParseXmlRpc(buffer, name, STR("/methodName"))) {
				if (name) name->Delete();
				return false;
			}

			static StaticStringObject method(STR("method"));
			ob->ToDict()->Insert(&method, name);
			continue;
		}

		if (tag.Equals(STR("params"))) {
			Object* value = NULL;
			if (!ParseXmlRpcParams(buffer, value)) {
				if (value) value->Delete();
				return false;
			}

			static StaticStringObject params(STR("params"));
			ob->ToDict()->Insert(&params, value);
			continue;
		}

		if (tag.Equals(STR("params /"))) {
			Object* value = ArrayObject::Alloc();  // empty array.
			static StaticStringObject params(STR("params"));
			ob->ToDict()->Insert(&params, value);
			continue;
		}

		ob->Delete();
		ob = NULL;

		return false;
	}
}


bool Object::ParseXmlRpcParams(Deque<uchar>& buffer, Object*& ob)
{
	ob = ArrayObject::Alloc();

	for (;;) {
		const String tag = ParseTag(buffer);

		if (tag.Equals(STR("/params")))  return true;
		if (!tag.Equals(STR("param"))) return false;

		if (!ParseTag(buffer).Equals(STR("value"))) return false;

		Object* value = NULL;
		if (!ParseXmlRpc(buffer, value))  return false;

		assert(value);
		ob->ToArray()->PushBack(value);

		if (!ParseTag(buffer).Equals(STR("/value"))) return false;
		if (!ParseTag(buffer).Equals(STR("/param"))) return false;
	}
}


// * static
void Object::ToJsonEquivalence(Object*& ob)
{
	if (!ob->Retrieve(STR("method"), STRING)) return;
	Object* params;
	if (!((params = ob->Retrieve(STR("params"), ARRAY)))) return;
	ArrayObject* a = params->ToArray();
	if (a->Size() < 1 || a->Size() > 2) return;
	if ((*a)[0]->GetType() != DICT) return;
	if (a->Size() == 2 && (*a)[1]->GetType() != NUMBER) return;

	// Promote dict
	ob->ToDict()->GetValue(STR("params")) = exch<Object*>((*a)[0], NullObject::Alloc());;

	// Promote or generate id
	if (a->Size() == 2) {
		ob->ToDict()->Insert(STR("id"), exch<Object*>((*a)[1], NullObject::Alloc()));
	} else {
		static uint xmlid;
		ob->ToDict()->Insert(STR("id"), NumberObject::Alloc(++xmlid));
	}

	a->Delete();
}


// * static
bool Object::MethodCallAsXmlRpc(Vector<uchar>& buffer)
{
	NumberObject* id = (NumberObject*)Retrieve(STR("id"), NUMBER);
	Object* params = Retrieve(STR("params"));
	StringObject* method = (StringObject*)Retrieve(STR("method"), STRING);

	if (!id || !params || !method)
		return false;

	buffer.PushBack(STR("<?xml version=\"1.0\"?><methodCall><methodName>"));
	method->AsXmlRpc(buffer);
	buffer.PushBack(STR("</methodName><params><param><value>"));
	params->AsXmlRpc(buffer);
	buffer.PushBack(STR("</value></param><param><value>"));
	id->AsXmlRpc(buffer);
	buffer.PushBack(STR("</value></param></params></methodCall>"));
	// Add a newline - some braindead implementations won't work otherwise
	buffer.PushBack((uchar)'\n');
	return true;
}


void Object::AsXmlRpcResultResponse(Vector<uchar>& buffer)
{
	buffer.PushBack(STR("<?xml version=\"1.0\"?><methodResponse>"));
	buffer.PushBack(STR("<params><param><value>"));
	AsXmlRpc(buffer);
	buffer.PushBack(STR("</value></param></params>"));
	// Add a newline - some braindead implementations won't work otherwise
	buffer.PushBack(STR("</methodResponse>\n"));
}


void Object::AsXmlRpcErrorResponse(Vector<uchar>& buffer)
{
	assert(GetType() == DICT);

	buffer.PushBack(STR("<?xml version=\"1.0\"?><methodResponse>"));
	buffer.PushBack(STR("<fault><value>"));
	AsXmlRpc(buffer);
	buffer.PushBack(STR("</value></fault>"));
	// Add a newline - some braindead implementations won't work otherwise
	buffer.PushBack(STR("</methodResponse>\n"));
}


// * static
bool NullObject::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	if (buffer.Size() >= 4 && !::memcmp(buffer + 0, "null", 4)) {
		buffer.Erase(0, 4);
		ob = Alloc();
		return true;
	}
	return false;
}


// * static
bool TrueObject::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	Util::Trim(buffer);

	if (buffer.Size() >= 4 && !::memcmp(buffer + 0, "true", 4)) {
		buffer.Erase(0, 4);
		ob = Alloc();
		return true;
	}
	return false;
}


// * static
bool TrueObject::ParseXmlRpc(Deque<uchar>& buffer, Object*& ob)
{
	Util::Trim(buffer);

	if (buffer.Size() >= 4 && !::memcmp(buffer + 0, "1", 4)) {
		buffer.Erase(0, 4);
		ob = TrueObject::Alloc();
	} else if (buffer.Size() >= 5 && !::memcmp(buffer + 0, "0", 5)) {
		buffer.Erase(0, 5);
		ob = FalseObject::Alloc();
	} else
		return false;

	return ParseTag(buffer).Equals(STR("/boolean"));
}


// * static
bool FalseObject::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	if (buffer.Size() >= 5 && !::memcmp(buffer + 0, "false", 5)) {
		buffer.Erase(0, 5);
		ob = Alloc();
		return true;
	}
	return false;
}


// * static
bool StringObject::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	if (buffer.Size() < 2 || buffer[0] != '"') return false;

	buffer.PopFront();

	Vector<uchar> result;

	while (!buffer.Empty() && buffer[0] != '"') {
		switch (buffer[0]) {
		case '\\':
			buffer.PopFront();
			switch (buffer[0]) {
			case 'r': result.PushBack(13); break;
			case 'n': result.PushBack(10); break;
			case 'b': result.PushBack(8); break;
			case 'f': result.PushBack(12); break;
			case 't': result.PushBack(9); break;
			case 'u': {
				if (buffer.Size() < 5) return false;
				uint c = Util::ParseHexDigit(buffer[1]) << 12;
				c += Util::ParseHexDigit(buffer[2]) << 8;
				c += Util::ParseHexDigit(buffer[3]) << 4;
				c += Util::ParseHexDigit(buffer[4]);
				buffer.Erase(0, 4);
				result.PushBack(c);
				break;
			}
			default:
				result.PushBack(buffer[0]);
				break;
			}
			break;
		case '"':
			break;
		default:
			result.PushBack(buffer[0]);
		}
		buffer.PopFront();
	}

	if (buffer[0] == '"')  buffer.PopFront();

	StringObject* sob = Alloc();
	sob->_v.Take(result);
	ob = sob;

	return true;
}


// * static
bool StringObject::ParseXmlRpc(Deque<uchar>& buffer, Object*& ob, const String& endtag)
{
	Vector<uchar> result;
	while (!buffer.Empty() && buffer[0] != '<') {
		if (buffer[0] == '&') {
			buffer.PopFront();
			if (buffer.Size() >= 3 && !::memcmp(buffer + 0, "lt;", 3)) {
				buffer.Erase(0, 3);
				result.PushBack('<');
			} else {
				if (buffer.Size() >= 4 && !::memcmp(buffer + 0, "amp;", 4))
					buffer.Erase(0, 4);

				result.PushBack('&');
			}
		} else {
			result.PushBack(buffer[0]);
			buffer.PopFront();
		}
	}

	ob = StringObject::Alloc(String().Take(result));

	return endtag.Empty() || ParseTag(buffer).Equals(endtag);
}


#if 0
const String StringObject::GetString() const
{
	Vector<uchar> result;

	for (uint i = 0; i < _v.Size(); ++i) {
		switch (_v[i]) {
		case '\\':
			if (++i == _v.Size()) {
				result.PushBack('\\');				
				goto done;
			}

			switch (_v[i]) {
			case 'r': result.PushBack(13); break;
			case 'n': result.PushBack(10); break;
			case 'b': result.PushBack(8); break;
			case 'f': result.PushBack(12); break;
			case 't': result.PushBack(9); break;
			case 'u': {
				if (i < Size() - 5)
					goto done;
				uint c = Util::ParseHexDigit(_v[i+1]) << 12;
				c += Util::ParseHexDigit(buffer[i+2]) << 8;
				c += Util::ParseHexDigit(buffer[i+3]) << 4;
				c += Util::ParseHexDigit(buffer[i+4]);
				i += 5;
				result.PushBack(c);
				break;
			}
			default:
				result.PushBack(_v[i]);
				break;
			}
			break;
		default:
			result.PushBack(_v[i]);
		}
		++i;
	}

done:
	return String().Take(result);
}
#endif


void StringObject::AsJson(Vector<uchar>& dest)
{
	dest.PushBack('"');

	for (uint i = 0; i < _v.Size(); ++i) {
		const uint c = _v[i];

		switch (c) {
		case '\r':  dest.PushBack((const uchar*)"\\r"); continue;
		case '\n':  dest.PushBack((const uchar*)"\\n"); continue;
		case 010:  dest.PushBack((const uchar*)"\\b"); continue;
		case 014:  dest.PushBack((const uchar*)"\\f"); continue;
		case '\\':  dest.PushBack((const uchar*)"\\\\"); continue;
		case 011:  dest.PushBack((const uchar*)"\\t"); continue;
		case '"': dest.PushBack((const uchar*)"\\\""); continue;
		default:
			break;
		}

		if (c < ' ' || c >= 127) {
			dest.PushBack((const uchar*)"\\u00");
			dest.PushBack(((const uchar*)"0123456789ABCDEF")[c / 16]);
			dest.PushBack(((const uchar*)"0123456789ABCDEF")[c & 15]);
			continue;
		}

		dest.PushBack(c);
	}

	dest.PushBack('"');
}


void StringObject::AsXmlRpc(Vector<uchar>& dest)
{
	for (uint i = 0; i < _v.Size(); ++i) {
		const uint c = _v[i];
		if (c == '<')  dest.PushBack((const uchar*)"&lt;");
		else if (c == '&')  dest.PushBack((const uchar*)"&amp;");
		else dest.PushBack(c);
	}
}


void NumberObject::AsJson(Vector<uchar>& dest)
{
	Util::AppendFmt(dest, (const uchar*)"%D", _v);
}


void NumberObject::AsXmlRpc(Vector<uchar>& dest)
{
	Util::AppendFmt(dest, (const uchar*)"<i4>%d</i4>", (uint32_t)_v);
}


// * static
bool NumberObject::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	// This is kinda half-assed in that "-" alone will parse to 0 (-0)...
	Util::Trim(buffer);

	if (!(buffer.Size() >= 1 && ((buffer[0] >= '0' && buffer[0] <= '9') ||
								 buffer[0] == '-')))
		return false;

	const bool neg = (buffer[0] == '-');
	if (neg) buffer.PopFront();

	int64_t result = 0;

	while (buffer.Size() >= 1 && buffer[0] >= '0' && buffer[0] <= '9') {
		result *= 10;
		result += buffer[0] - '0';
		buffer.PopFront();
	}

	NumberObject* nob = Alloc(neg ? -result : result);
	ob = nob;

	return true;
}


// * static
bool NumberObject::ParseXmlRpc(Deque<uchar>& buffer, Object*& ob, const String& endtag)
{
	if (!ParseJson(buffer, ob))  return false;

	return ParseTag(buffer).Equals(endtag);
}


DictObject::~DictObject()
{
	while (!_v.Empty())
		Erase(_v.Size() - 1);
}


void DictObject::AsJson(Vector<uchar>& dest)
{
	dest.PushBack((const uchar*)"{");

	for (uint i = 0; i < _v.Size(); ++i) {
		dest.PushBack(' ');
		_v[i]->key->AsJson(dest);
		dest.PushBack((const uchar*)" : ");
		_v[i]->value->AsJson(dest);

		if (i < _v.Size() - 1)  dest.PushBack(',');
	}

	dest.PushBack((const uchar*)" }");
}


void DictObject::AsXmlRpc(Vector<uchar>& dest)
{
	// First try to serialize as a method call using Fling.IO convention
	if (Object::MethodCallAsXmlRpc(dest)) return;

	dest.PushBack(STR("<struct>"));
	
	for (uint i = 0; i < _v.Size(); ++i) {
		assert(_v[i]->key);
		assert(_v[i]->value);
		if (_v[i]->key->GetType() != Object::STRING) continue;

		dest.PushBack(STR("<member><name>"));
		_v[i]->key->AsXmlRpc(dest);
		dest.PushBack(STR("</name><value>"));
		_v[i]->value->AsXmlRpc(dest);
		dest.PushBack(STR("</value></member>"));
	}
	dest.PushBack(STR("</struct>"));
}


// * static
bool DictObject::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	if (buffer.Size() < 2 || buffer[0] != '{') return false;

	buffer.PopFront();
	Util::Trim(buffer);

	DictObject* dob = Alloc();

	while (!buffer.Empty() && buffer[0] != '}') {
		Object* key = NULL;
		Object* value = NULL;

		Util::Trim(buffer);
		if (!StringObject::ParseJson(buffer, key)) break;

		Util::Trim(buffer);
		if (buffer[0] != ':') {
			if (key) key->Delete();
			break;
		}
		buffer.PopFront();
		Util::Trim(buffer);
		if (!Object::ParseJson(buffer, value)) {
			if (key) key->Delete();
			break;
		}

		dob->Insert(key, value);

		Util::Trim(buffer);
		if (!buffer.Empty() && buffer[0] == ',') {
			buffer.PopFront();
			Util::Trim(buffer);
		}
	}

	Util::Trim(buffer);
	if (!buffer.Empty() && buffer[0] == '}')  buffer.PopFront();

	ob = dob;
	return true;
}


// * static
bool DictObject::ParseXmlRpc(Deque<uchar>& buffer, Object*& ob)
{
	ob = DictObject::Alloc();

	Object* key;
	Object* value;

	for (;;) {
		key = NULL;
		value = NULL;

		Util::Trim(buffer);

		if (buffer.Size() < 9)  return false;
		if (!::memcmp(buffer + 0, "</struct>", 9))  {
			ParseTag(buffer);	// remove tag
			return true;
		}

		if (!ParseTag(buffer).Equals(STR("member"))) return false;

		if (!ParseTag(buffer).Equals(STR("name"))) return false;
		if (!StringObject::ParseXmlRpc(buffer, key, STR("/name"))) goto error;

		if (!ParseTag(buffer).Equals(STR("value"))) goto error;
		if (!Object::ParseXmlRpc(buffer, value)) goto error;
		if (!ParseTag(buffer).Equals(STR("/value"))) goto error;
		if (!ParseTag(buffer).Equals(STR("/member"))) goto error;

		ob->ToDict()->Insert(key, value);

	}

error:
	if (key) key->Delete();
	if (value) value->Delete();

	return false;
}


void DictObject::Insert(Object* key, Object* value)
{
	Erase(key);
	KV* kv = _kvf.Alloc();
	kv->key = key;
	kv->value = value;
	_v.Insert(kv);
}


void DictObject::Insert(const uchar* key, Object* value)
{
	Insert(StringObject::Alloc(key), value);
}


void DictObject::Erase(Object* key)
{
	if (key->GetType() != Object::STRING)  return;

	Erase(key->ToString()->GetString().CStr());
}


Object*& DictObject::GetValue(const String& key)
{
	// It's probably not worth the overhead to use bisect here
	for (uint i = 0; i < _v.Size(); ++i) {
		if (_v[i]->key->GetType() == Object::STRING &&
			_v[i]->key->ToString()->GetString() == key)
			return _v[i]->value;
	}

	static Object* tmpob = NULL;
	assert(!tmpob);				// Detect if it got accidentally modified...
	return tmpob;
}


void DictObject::Erase(const uchar* key)
{
	const String tmp(key);
	for (uint i = 0; i < _v.Size(); ++i) {
		if (_v[i]->key->GetType() == Object::STRING &&
			_v[i]->key->ToString()->GetString() == tmp) {
			Erase(i);
			return;
		}
	}
}


void DictObject::Erase(uint pos)
{
	assert(pos <= _v.Size());
	_v[pos]->DeleteObjects();
	_kvf.Free(_v[pos]);
	_v.Erase(pos);
}


Object* DictObject::Clone()
{
	DictObject* d = Alloc();

	for (uint i = 0; i < _v.Size(); ++i) {
		KV* kv = _kvf.Alloc();
		kv->key = _v[i]->key->Clone();
		kv->value = _v[i]->value->Clone();
		d->_v.Insert(kv);
	}

	return d;
}


ArrayObject::~ArrayObject()
{
	_v.DeleteObjects();
}


void ArrayObject::AsJson(Vector<uchar>& dest)
{
	dest.PushBack(STR("["));

	for (uint i = 0; i < _v.Size(); ++i) {
		dest.PushBack(' ');
		_v[i]->AsJson(dest);

		if (i < _v.Size() - 1)  dest.PushBack(',');
	}

	dest.PushBack(STR(" ]"));
}


void ArrayObject::AsXmlRpc(Vector<uchar>& dest)
{
	dest.PushBack((const uchar*)"<array>");
	for (uint i = 0; i < _v.Size(); ++i) {
		assert(_v[i]);
		dest.PushBack((const uchar*)"<value>");
		_v[i]->AsXmlRpc(dest);
		dest.PushBack((const uchar*)"</value>");
	}
	dest.PushBack((const uchar*)"</array>");
}


// * static
bool ArrayObject::ParseJson(Deque<uchar>& buffer, Object*& ob)
{
	if (buffer.Size() < 2 || buffer[0] != '[') return false;

	buffer.PopFront();
	Util::Trim(buffer);

	ArrayObject* aob = Alloc();

	while (!buffer.Empty() && buffer[0] != ']') {
		Object* value;

		Util::Trim(buffer);
		if (!Object::ParseJson(buffer, value)) break;

		aob->PushBack(value);

		Util::Trim(buffer);
		if (!buffer.Empty() && buffer[0] == ',') {
			buffer.PopFront();
			Util::Trim(buffer);
		}
	}

	Util::Trim(buffer);
	if (!buffer.Empty() && buffer[0] == ']')  buffer.PopFront();

	ob = aob;
	return true;
}


// * static
bool ArrayObject::ParseXmlRpc(Deque<uchar>& buffer, Object*& ob)
{
	ob = ArrayObject::Alloc();
	Object* value;

	for (;;) {
		value = NULL;

		Util::Trim(buffer);

		if (buffer.Size() < 7)  return false;
		if (!::memcmp(buffer + 0, "</data>", 7))  {
			ParseTag(buffer);	// Skip </data>
			return ParseTag(buffer).Equals(STR("/array"));
		}

		if (!ParseTag(buffer).Equals(STR("value"))) return false;
		if (!Object::ParseXmlRpc(buffer, value)) goto error;
		if (!ParseTag(buffer).Equals(STR("/value"))) goto error;

		ob->ToArray()->PushBack(value);
	}

error:
	if (value) value->Delete();

	return false;
}



Object* ArrayObject::Clone()
{
	ArrayObject* a = Alloc(Size());

	for (uint i = 0; i < _v.Size(); ++i)
		a->PushBack((*this)[i]->Clone());

	return a;
}


Object* Object::Retrieve(const String& name, Type expected_type, const String& div)
{
	Vector<String*> names;
	name.Split(names, div);

	Object* node = this;
	uint i = 0;

	while (node) {
		if (i == names.Size()) {
			if (expected_type == ANY_TYPE || node->GetType() == expected_type)
				break;
			node = NULL;
		} else {
			if (node->GetType() != DICT)  {
				node = NULL;
				break;
			}
			assert(names[i]);
			node = node->ToDict()->GetValue(*names[i]);
			++i;
		}
	}

	names.DeleteObjects();
	return node;
}
