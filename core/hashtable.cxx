// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "hashtable.h"


HashTable::HashTable(HashTable::HashFunc& hf, HashTable::CompareFunc& cf, uint reserve, uint d) :
	_hash(hf), _compare(cf)
{
	assert(_hash);
	_eviction_depth = d;
	_v.Grow(reserve);

	::memset(_v + 0, 0, reserve * sizeof(void*));
#ifdef DEBUG
	_num_resizes = 0;
	_worst_occupancy = ~0;
	_best_occupancy = 0;
#endif
}


void HashTable::SetEvictionDepth(uint newd) 
{
    ScopedNoInt G;

	_eviction_depth = newd;
}


uint HashTable::Insert(void* item) 
{
    ScopedNoInt G;

	uint mask = _v.Size() - 1;
	uint32_t h1 = 0;
	uint32_t h2 = 0;
	_hash(item, h1, h2);
	h1 &= mask;
	h2 &= mask;
	if (_v[h1] && _compare(_v[h1], item)) return h1;
	if (_v[h2] && _compare(_v[h2], item)) return h2;

	for (;;) {
		uint32_t pos = h1;
		for (uint n = 0; n < _eviction_depth; ++n) {
			if (!_v[pos]) { _v[pos] = item; return pos; }
			item = exch<void*>(_v[pos], item);
			h1 = h2 = 0;
			_hash(item, h1, h2);
			h1 &= mask;
			h2 &= mask;
			pos = (pos == h1 ? h2 : h1);
		}

		Rehash();

		mask = _v.Size() - 1;
		h1 = h2 = 0;
		_hash(item, h1, h2);
		h1 &= mask;
		h2 &= mask;
	}
}
	
uint HashTable::Find(const void* arg) const
{
    ScopedNoInt G;

	const uint mask = _v.Size() - 1;
	uint32_t h1 = 0;
	uint32_t h2 = 0;
	_hash(arg, h1, h2);
	h1 &= mask;
	h2 &= mask;
	if (_v[h1] && _compare(_v[h1], arg)) return h1;
	if (_v[h2] && _compare(_v[h2], arg)) return h2;
	return NOT_FOUND;
}


void HashTable::Erase(const uint pos) 
{
    ScopedNoInt G;

	_v[pos] = NULL;
}


uint HashTable::Size() const 
{
    ScopedNoInt G;
	return _v.Size();
}


void* HashTable::operator[](const uint arg) {
    ScopedNoInt G;
	return _v[arg];
}


// Stats
uint HashTable::GetNumUsed() const
{
    ScopedNoInt G;

	uint num_items = 0;

	for (uint n = 0; n < _v.Size(); ++n)
		if (_v[n]) ++num_items;

	return num_items;
}


void HashTable::Rehash()
{
#ifdef DEBUG
	++_num_resizes;
	const uint occupancy = GetOccupancy();
	_worst_occupancy = min(occupancy, _worst_occupancy);
	_best_occupancy = max(occupancy, _best_occupancy);
//	DMSG("Rehashing: size: %u, occupancy is %u%%", _v.Size(), occupancy);
#endif
	Vector<void*> _prev; _prev.Take(_v);

	_v.Clear();
	const uint new_size = _prev.Size() * 2;
	_v.Grow(new_size);
	::memset(_v + 0, 0, new_size * sizeof(void*));

	for (uint i = 0; i < _prev.Size(); ++i) {
		if (_prev[i])
			Insert(_prev[i]);
	}
}
