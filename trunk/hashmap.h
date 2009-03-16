#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "hashtable.h"

template <typename K, typename V,
		  HashTable::HashFunc KeyHash = K::Hash,
		  HashTable::CompareFunc KeyComp = K::HashEqual>
class HashMap {
	HashTable _table;
public:
	struct KV {
		K _key;
		V _value;

		KV(const K& key, const V& value) : _key(key), _value(value) { }

		static void Hash(const void* v, uint32_t& hash1, uint32_t& hash2) {
			assert(v);
			KV& kv = *(KV*)v;
			KeyHash(&kv._key, hash1, hash2);
		}

		static bool HashEqual(const void* v1, const void* v2) {
			assert(v1);
			assert(v2);
			KV& kv1 = *(KV*)v1;
			KV& kv2 = *(KV*)v2;
			return KeyComp(&kv1._key, &kv2._key);
		}
	};

	HashMap(uint reserve = HASHTABLE_DEFAULT_RESERVE,
			uint depth = HASHTABLE_EVICTION_DEPTH) :
		_table(KV::Hash, KV::HashEqual, reserve, depth)
	{ }

	~HashMap() {
		for (uint i = 0; i < _table.Size(); ++i)  delete (KV*)_table[i];
	}

	V& operator[](const K& key) {
		uint pos = Lookup(key);
		if (pos == NOT_FOUND)  pos = _table.Insert(new KV(key, V()));
		return ((KV*)_table[pos])->_value;
	}

	bool Erase(const K& key) {
		const uint pos = Lookup(key);
		if (pos == NOT_FOUND) return false;
		delete (KV*)_table[pos];
		_table.Erase(pos);
		return true;
	}

	bool Insert(const K& key, const V& value) {
		uint pos = Lookup(key);
		bool added = false;
		if (pos == NOT_FOUND) {
			pos = _table.Insert(new KV(key, value));
			added = true;
		} else {
			((KV*)_table[pos])->_value = value;
		}

		return added;
	}

	bool Find(const K& key, V& value) {
		const uint pos = Lookup(key);
		if (pos == NOT_FOUND) return false;
		value = ((KV*)_table[pos])->_value;
		return true;
	}

private:
	uint Lookup(const K& key) const {
		KV dummy(key, V());
		return _table.Find(&dummy);
	}

	HashMap(const HashMap&);
	HashMap& operator=(const HashMap&);
};


#endif // __HASHMAP_H__
