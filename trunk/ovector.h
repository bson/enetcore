#ifndef __OVECTOR_H__
#define __OVECTOR_H__


// Ordered vector
// This differs from a set in that the same item can reoccur.
// The only way to add items is using Insert()
// Like STL, requires T to implement bool operator>(const T&) const

// Greater items insert towards the back, lesser items towards the front.
// Equal items sort by insertion order.

// Safe for POD only

template <typename T, OrderFunc Order = T::Order> 
class OVector {
	Vector<T> _v;

	typedef OVector<T,Order> Self;

public:
	OVector() { }
	OVector(uint reserve) : _v(reserve) { }

	Self& assign(const Self& arg) { _v.assign(arg._v); return *this; }

	OVector(const Self& arg) : _v(arg._v) {  }
	Self& operator=(const Self& arg) { return assign(arg); }
	
	virtual ~OVector() { }

	/// These are all the same as Vector<>

	T* Grab(uint& used, uint& start, uint& alloc) { return _v.Grab(used, start, alloc); }

	void Reserve(uint new_size) { _v.Reserve(new_size); }
	uint GetReserve() const { return _v.GetReserve(); }
	uint Grow(uint num) { return _v.Grow(num); }
	uint Headroom() const { return _v.Headroom(); }
	void SetSize(uint arg) { _v.SetSize(arg); }
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

	uint Insert(const T& arg, uint start = 0) {
		const uint pos = FindGreaterThan(arg, start);
		_v.Insert(pos) = arg;
		return pos;
	}

	void DeleteEntries() { _v.DeleteEntries(); }
	void DeleteObjects() { _v.DeleteObjects(); }

	static bool IsEqual(const T* a, const T* b) { return !Order(a, b) && !Order(b, a); }

	// Return first item greater than arg.  If arg is the greatest,
	// returns size of vector.  If arg is smaller than item 0, returns
	// 0.  Basically, find the insertion point.
	uint FindGreaterThan(const T& arg, int start = 0) {
		assert(start >= 0);

		int lo = start;
		int hi = Size();
		int bisect = hi;

		while (lo < hi) {
			bisect = (lo + hi) / 2;
			if (Order(&arg, _v + bisect))
				lo = bisect + 1; // arg > _v[bisect]
			else if (Order(_v + bisect, &arg))
				hi = bisect;	// arg < _v[bisect]
			else
				break;			// arg == bisect
		}

		// Return first item GT
		while (bisect < _v.Size() && !Order(_v + bisect, &arg))
			++bisect;

		return bisect;
	}

	uint Find(const T& arg, uint start = 0) {
		if (start >= _v.Size()) return NOT_FOUND;

		const uint pos = FindGreaterThan(arg, start);
		if (pos == start || !IsEqual(&arg, _v + (pos - 1)))
			return NOT_FOUND;
		return pos - 1;
	}

	// For ODeque::Insert
	T& InsertInternal(uint pos) { return _v.Insert(pos); }
};

#endif // __OVECTOR_H__
