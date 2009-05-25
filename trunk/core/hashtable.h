#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

// This implements a Cuckoo Hash:
// An key is hash to two slots.
// If the first slot is occupied, the second is used.
// If the second slot is unoccupied, it's used.
// Otherwise, if both slots are in use, the occupant of the first is evicted;
// this is where the algorithm gets its name.
// The evicted occupant is then reinserted.
// The process then repeats as necessary.
//
// One way to end is on recursion.
// Another is to limit the number of evictions.
// We go for the latter since it makes for easier analysis; the cost of rehash
// becomes O(1/n) where n is our upper bound on evictions before giving up.
//
// In pseudo code (from http://www.it-c.dk/people/pagh/papers/cuckoo-undergrad.pdf):
//
// procedure insert(x) 
//   if T [h1(x)] = x or T[h2 (x)] = x then return; 
//   pos ← h1(x); 
//   loop n times 
//   { 
// 	    if T [pos] = NULL then { T [pos] ← x; return}; 
// 	    x ↔ T [pos]; 
// 	    if pos= h1 (x) then pos ← h2 (x) else pos ← h1(x); 
//   } 
//   rehash(); insert(x)
// end 

// The hash table is MT-safe.

template <typename T> bool HashEqual(const void* va, const void* vb) {
	const T& a = *(T*)va;
	const T& b = *(T*)vb;
	return a == b;
}


class HashTable {
public:
	// Hash object
	typedef void (HashFunc)(const void* key, uint32_t& hash1, uint32_t& hash2);

	// Test for equality
	typedef bool (CompareFunc)(const void* key1, const void* key2);

private:
	// This trades off footprint for speed
	// For 5000 random NetAddr's hashed with Lookup3 (NetAddr::Hash), we get
	//
	// depth  best  worst  rehashes
	//   4     60%    16%      9
	//   8     75%    38%      8
	//  10     80%    48%      8
	//  12     92%    64%      7
	//  16     95%    59%      8
	//  20     95%    57%      7
	//  32     97%    78%      7
    // 128     99%    96%      7
	//
	// Complexity is O(n), so grows linearly with depth.  The larger the table, the
	// longer the depth should generally be.  Probably in the range [size/512, size/100].
	// Because this depends on usage pattern, the value is a parameter.
	// It can even be changed using SetEvictionDepth().

	mutable Spinlock _lock;

	Vector<void*> _v;
	uint _eviction_depth;
	HashFunc& _hash;
	CompareFunc& _compare;

#ifdef DEBUG
	uint _num_resizes;
	uint _worst_occupancy;		// in %
	uint _best_occupancy;		// in %
#endif

public:
	HashTable(HashFunc& hf, CompareFunc& cf, uint reserve = HASHTABLE_DEFAULT_RESERVE,
			  uint d = HASHTABLE_EVICTION_DEPTH);

	void SetEvictionDepth(uint newd);
	uint Insert(void* arg);
	uint Find(const void* arg) const;
	void Erase(const uint pos);
	uint Size() const;

	template <typename T> void DeleteEntries() { 
		for (uint i = 0; i < _v.Size(); ++i)  delete (T)_v[i];
	}

	void* operator[](const uint arg);

	// Stats
	uint GetNumUsed() const;

#ifdef DEBUG
	uint GetNumResizes() const { return _num_resizes; }
	uint GetBestOccupancy() const { return _best_occupancy; }
	uint GetWorstOccupancy() const { return _worst_occupancy; }

	uint GetOccupancy() {
		Spinlock::Scoped L(_lock);
		return GetNumUsed() * 100 / _v.Size();
	}
#endif

private:

	void Rehash();
};

#endif // __HASHTABLE_H__
