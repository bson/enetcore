#ifndef __SET_H__
#define __SET_H__



// Set: an ordered vector that guarantees uniqueness
// Safe for POD only

template <typename T, OrderFunc Order = T::Order> 
class Set {
	OVector<T,Order> _v;

	typedef Set<T,Order> Self;

public:
	Set() { }
	Set(uint reserve) : _v(reserve) { }

	Self& assign(const Self& arg) { _v.assign(arg._v); return *this; }

	Set(const Self& arg) : _v(arg._v) { }
	Self& operator=(const Self& arg) { return assign(arg); }

	virtual ~Set() { }

	/// These are all the same as OVector<>

	void SetAutoResize(bool flag) { _v.SetAutoResize(flag); }
	void Reserve(uint new_size) { _v.Reserve(new_size); }
	void Grow(uint num) { _v.Grow(num); }
	uint Headroom() const { return _v.Headroom(); }
	bool Empty() const { return _v.Empty(); }
	uint Size() const { return _v.Size(); }
	void Clear() { _v.Clear(); }
	T& Front() { return _v.Front(); }
	const T& Front() const { return _v.Front(); }
	T& Back() { return _v.Back(); }
	const T& Back() const { return _v.Back(); }
	T& operator[](uint arg) { return _v[arg]; }
	const T& operator[](uint arg) const { return _v[arg]; }
	T* operator+(uint arg) { return _v + arg; }
	const T* operator+(uint arg) const { return _v + arg; }
	void Erase(uint pos, uint num = 1) { _v.Erase(pos, num); }
	void PopFront() { Erase(0, 1); }
	void PopBack() { Erase(Size() - 1, 1); }

	/// These differ

	// Test if two T's are equal
	bool IsEqual(const T& a, const T& b) const { return !Order(&a, &b) && !Order(&b, &a); }

	uint Insert(const T& arg) {
		const uint pos = _v.FindGreaterThan(arg);
		if (pos && IsEqual(arg, _v[pos-1])) {
			// Already have an entry for it - replace
			_v[pos-1] = arg;
		} else {
			// New item
			_v.InsertInternal(pos) = arg;
		}
		return pos;
	}

	void DeleteEntries() { _v.DeleteEntries(); }
	void DeleteObjects() { _v.DeleteObjects(); }

	void Erase(const T& arg) {
		const uint pos = Find(arg);
		if (pos != NOT_FOUND)  Erase(pos, 1);
	}

	uint Find(const T& arg) {
		const uint pos = _v.FindGreaterThan(arg);
		if (!pos || !IsEqual(arg, _v[pos - 1])) return NOT_FOUND;
		return pos - 1;
	}
};


// A KeyValue pair
// Like an STL pair, except elements are called 'key' and 'value', and order
// by key (usually).

template <typename K, typename V, OrderFunc KeyOrder = K::Order>
struct KeyValue {
	typedef KeyValue<K,V,KeyOrder> Self;

	K key;
	V value;

	KeyValue() { }
	KeyValue(K k) : key(k) { }
	KeyValue(K k, V v) : key(k), value(v) { }
	virtual ~KeyValue() { }

	KeyValue(const Self& arg) : key(arg.key), value(arg.value) { }

	Self& assign(const Self& arg) {
		if (&arg != this) { key = arg.key; value = arg.value; }
		return *this;
	}

	Self& operator=(const Self& arg) { return assign(arg); }

	void DeleteEntries() { delete key; delete value; }
	void DeleteObjects() { if (key) key->Delete(); if (value) value->Delete(); }

	// For OContainer<KeyValue*>
	static bool Order(const void* a, const void* b) {
		const Self& kv1 = **(Self**)a;
		const Self& kv2 = **(Self**)b;

		return KeyOrder(&kv1.key, &kv2.key);
	}
};


#endif // __SET_H__
