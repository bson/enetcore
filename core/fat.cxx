// Copyright (c) 2018 Jan Brittenson
// See LICENSE for details.

#include "enetkit.h"
#include "fat.h"
#include "util.h"


Fat::Fat(BlockDev& dev) :
	_dev(dev)
{
}


bool Fat::Mount(uint partnum, bool rw)
{
	_dev.Retain();

	_rw = rw;

	if (!_dev.Init()) {
        console("FAT: failed to mount device");
        _dev.Release();
        return false;
    }

	uint8_t* sector = (uint8_t*)xmalloc(514);
	const PartEnt* part = (PartEnt*)(sector + 448);
	bool ok = false;
	uint8_t type;

	// Read MBR
	if (!_dev.ReadSector(0, sector + 2)) goto failed;
	if (*(uint16_t*)(sector + 512) != LE16(0xaa55)) goto failed;

	_lba0 = LE32(part[partnum].lba_begin);
	_size = LE32(part[partnum].num_sect);

	if (!_lba0 || !_size) goto failed;

	_fat32 = false;

	type = part[partnum].type_code;

	if (type == 0xb || type == 0xc) _fat32 = true;
	else if (type != 6 && type != 4)  goto failed;

	// Read volume ID
	if (!_dev.ReadSector(_lba0, sector)) goto failed;
	if (*(uint16_t*)(sector + 510) != LE16(0xaa55)) goto failed;

	uint16_t byte_per_sect;
	memcpy(&byte_per_sect, sector + VID_Bytes_Per_Sec, sizeof byte_per_sect);
	if (LE16(byte_per_sect) != 512) goto failed;

	uint16_t tmp;
	if (_fat32) {
		memcpy(&_fat_num_sect, sector + VID_Sect_Per_FAT, sizeof _fat_num_sect);
		_fat_num_sect = LE32(_fat_num_sect);
	} else {
		memcpy(&tmp, sector + VID16_Sect_Per_FAT, sizeof tmp);
		_fat_num_sect = LE16(tmp);
	}

	// This is for FAT32; the value is calculated later for FAT16
	memcpy(&_root_dir_clus, sector + VID_Root_Dir_Clus, sizeof _root_dir_clus);
	_root_dir_clus = LE32(_root_dir_clus);

	memcpy(&_resv_sect, sector + VID_Res_Sects, sizeof _resv_sect);
	_resv_sect = LE16(_resv_sect);

	_sec_per_clus = sector[VID_Sect_Per_Clus];
	_clus_bits = Util::ffs(_sec_per_clus);

	if (!_fat_num_sect || _sec_per_clus != (1 << _clus_bits) ||
		(_fat32 && !IsValidCluster(_root_dir_clus)) || sector[VID_Num_FATs] != 2)
		goto failed;

	_fat_sector = _lba0 + _resv_sect;
	_cluster0 = _fat_sector + 2 * _fat_num_sect;

	// FAT16: advance _cluster0 past root dir and fill in _root_dir_clus
	_max_root = 0;
	if (!_fat32) {
		memcpy(&_max_root, sector + VID_Max_Root_Dir, 2);
		_max_root = LE16(_max_root);
		_root_dir_clus = (uint32_t)-(_max_root * 32 / 512);
		_cluster0 += _max_root * 32 / 512;
	}

	ok = true;

	{
		const String vol((uint8_t*)(sector + (_fat32 ? VID_Volume_Name : VID16_Volume_Name)), 11);
		console("sd0.%u: %S %uMB (FAT%u, %uk)",
				partnum,
				&vol,
				_size / 2 / 1024,
				_fat32 ? 32 : 16, 
				_sec_per_clus / 2);
	}

failed:
	xfree(sector);

	_dev.Release();
	return ok;
}


// * static
void Fat::FTWalker::NameFrom83(Vector<uchar>& sfn, const uint8_t* dirbuf)
{
	uint last_nonspace = NOT_FOUND;
	for (uint i = 0; i < 8; ++i) {
		if (dirbuf[i] != ' ') last_nonspace = i;
		sfn.PushBack(dirbuf[i]);
	}

	if (last_nonspace != NOT_FOUND)  sfn.SetSize(last_nonspace + 1);
	
	sfn.PushBack((uchar)'.');
	
	last_nonspace = NOT_FOUND;
	for (uint i = 8; i < 11; ++i) {
		sfn.PushBack(dirbuf[i]);
		if (dirbuf[i] != ' ') last_nonspace = sfn.Size();
	}

	if (last_nonspace != NOT_FOUND)  sfn.SetSize(last_nonspace);
}


template <typename Cluster>
bool Fat::GetFileClustersImpl(Vector<uint32_t>& clusters, uint32_t cluster1)
{
	uint32_t fatsec = NOT_FOUND; // FAT Sector we currently have loaded
	uint8_t* fat = (uint8_t*)xmalloc(512); // FAT Sector buffer

	bool success = false;

	enum { NUM_FAT_PER_SEC = 512 / sizeof (Cluster) };

	Cluster clus = cluster1 & 0x0fffffff;

	while (clus && clus < (Cluster)0x0ffffff0) {
		const uint32_t next_fatsec = clus / NUM_FAT_PER_SEC;
		if (next_fatsec >= _fat_num_sect) goto done;

		clusters.PushBack((uint32_t)clus);

		if (fatsec == NOT_FOUND || fatsec != next_fatsec) {
			// Need different FAT sector - load it
			if (!_dev.ReadSector(_fat_sector + next_fatsec, fat)) goto done;
			fatsec = next_fatsec;
		}

		clus = ((const Cluster*)fat)[clus & (NUM_FAT_PER_SEC - 1)];
		clus &= 0x0fffffff;
	}

	// If we ran into anything other than an end-of-chain marker we have
	// a corrupt FAT.
	success = clus >= (Cluster)0x0ffffff8;

done:
	if (!success)  console("Corrupt FAT/DIR entry: end of chain 0x%08x", (uint)clus);

	xfree(fat);
	return success;
}


bool Fat::GetFileClusters(Vector<uint32_t>& clusters, uint32_t cluster1)
{
	return _fat32 ? GetFileClustersImpl<uint32_t>(clusters, cluster1) :
		GetFileClustersImpl<uint16_t>(clusters, cluster1);
}


bool Fat::LoadDataCluster(Vector<uint8_t>& buffer, uint32_t cluster)
{
	assert(cluster >= 2);
	assert(cluster < 0x0ffffff0);

	uint32_t sector1 = ClusterToSector(cluster - 2);
	for (uint32_t sector = sector1; sector < sector1 + ClusterToSector(1); ++sector) {
		if (!LoadDataSector(buffer + buffer.Grow(512), sector))
			return false;
	}
	return true;
}


bool Fat::LoadDataClusters(Vector<uint8_t>& buffer, Vector<uint32_t>& cluster_list)
{
	for (uint i = 0; i < cluster_list.Size(); ++i) {
		if (!LoadDataCluster(buffer, cluster_list[i]))
			return false;
	}
	return true;
}


bool Fat::LoadDataSector(uint8_t* buf, uint32_t sector)
{
	return _dev.ReadSector(_cluster0 + sector, buf);
}


bool Fat::LoadDataSectors(Vector<uint8_t>& buf, uint32_t sector, uint num_sectors)
{
	while (num_sectors--) {
		if (!_dev.ReadSector(_cluster0 + sector, buf + buf.Grow(512)))
			return false;
		++sector;
	}
	return true;
}


Fat::FTWalker::FTWalker(Fat& fat) :
	_fat(fat)
{
	Reset();
}


bool Fat::FTWalker::Find(const String& path, bool enclosing, String& enclosed)
{
	Vector<String*> pathlist;
	path.Split(pathlist, STR("/"));
	assert_bounds(!pathlist.Empty());

	bool ok = false;

	const uint depth = pathlist.Size() - (enclosing ? 1 : 0);
	if (!depth)  {
		// Looking for this directory
		ok = true;
		goto done;
	}

	for (uint i = 0; i < depth; ++i) {
		const uint entry = FindNext(*pathlist[i]);
		if (entry == NOT_FOUND) goto done;
		if (!GetDirEnt(entry)->IsDir()) goto done;
		Load(entry);			// Descend
	}

	ok = true;
done:
	if (ok && enclosing) enclosed = *pathlist.Back();
	pathlist.DeleteObjects();
	return ok;
}


bool Fat::FTWalker::Reset()
{
	_dir.Clear();

	if (_fat._fat32) {
		Vector<uint32_t> dir_clusters;

		if (!_fat.GetFileClusters(dir_clusters, _fat._root_dir_clus)) return false;
		if (!_fat.LoadDataClusters(_dir, dir_clusters)) return false;
	} else {
		// FAT16: root dir is sector #
		if (!_fat.LoadDataSectors(_dir, _fat._root_dir_clus, _fat._max_root * 32 / 512))
			return false;
	}

	return true;
}


bool Fat::FTWalker::Load(uint entry)
{
	assert(entry != NOT_FOUND);

	const uint32_t cluster1 = GetDirEnt(entry)->GetCluster();

	Vector<uint32_t> dir_clusters;

	if (!_fat.GetFileClusters(dir_clusters, cluster1)) return false;
	if (!_fat.LoadDataClusters(_dir, dir_clusters)) return false;

	return true;
}


uint Fat::FTWalker::FindNext(const String& name, uint start)
{
	if (_dir.Empty() || start == Size())  return NOT_FOUND;

	assert(start == NOT_FOUND || (int)start >= 0);
	assert(start == NOT_FOUND || start < Size());

	for (uint entry = start != NOT_FOUND ? start + 1 : 0; entry < Size(); ++entry) {
		const FatDirEnt* d = GetDirEnt(entry);

		if (!d->IsUsed()) continue;
		if (d->IsLFN()) continue;
		if (d->IsVolume()) continue;

		String fn;

		// Compare in order: LFN, SFN
		if (name.Empty() || (GetLFN(fn, entry) && fn.Equals(name))) return entry;
		if (GetSFN(fn, entry) && fn.Equals(name)) return entry;
	}

	return NOT_FOUND;
}


bool Fat::FTWalker::GetLFN(String& lfn, uint entry) const
{
	assert(entry != NOT_FOUND);

	Vector<uchar> fn;			// Collects LFN
	fn.Reserve(256);
	fn.SetAutoResize(false);

	while ((int)entry >= 0) {

		const FatDirEnt* d = GetDirEnt(entry);

		if (!d->IsLFN()) break;

		// XXX maybe check dirent LFN ordinal?  Nah...

		if (fn.Headroom() < 13) return false; // Egads, broken DIR

		const uint8_t* p = (const uint8_t*)d;
		++p;
		for (uint i = 0; i < 5; ++i, p += 2)  fn.PushBack(*p);
		p += 3;
		for (uint i = 0; i < 6; ++i, p += 2)  fn.PushBack(*p);
		p += 2;
		fn.PushBack(*p);
		p += 2;
		fn.PushBack(*p);

		--entry;
	}

	// Remove trailing LFN spaces
	int i;
	for (i = fn.Size() - 1; i >= 0; --i) {
		if (fn[i] != ' ') break;
	}
	fn.SetSize(i+1);

	lfn.Take(fn);

	return true;
}


bool Fat::FTWalker::GetSFN(String& sfn, uint entry) const
{
	const FatDirEnt* d = GetDirEnt(entry);

	Vector<uchar> v;
	NameFrom83(v, d->name);
	sfn.Take(v);
	return true;
}


FatDirEnt* Fat::FTWalker::GetDirEnt(uint entry)
{
	assert(entry != NOT_FOUND);
	if (entry * sizeof (FatDirEnt) >= _dir.Size()) return NULL;

	return (FatDirEnt*)(_dir + 0) + entry;
}


const FatDirEnt* Fat::FTWalker::GetDirEnt(uint entry) const
{
	assert(entry != NOT_FOUND);
	if (entry * sizeof (FatDirEnt) >= _dir.Size()) return NULL;

	return (FatDirEnt*)(_dir + 0) + entry;
}


FatFile* Fat::Open(const String& path)
{
	_dev.Retain();

	FatFile* file = NULL;

	FTWalker ftw(*this);
	String filename;
	if (ftw.Find(path, true, filename)) {
		const uint entry = ftw.FindNext(filename);
		if (entry != NOT_FOUND) {
			file = new FatFile(*this);

			const FatDirEnt* dirent = ftw.GetDirEnt(entry);
			file->_size = dirent->GetFileSize();
			file->_clusters.Reserve(SectorToCluster((file->_size + 511) / 512) + 1);

			if (!GetFileClusters(file->_clusters, dirent->GetCluster()))
				delete exch<FatFile*>(file, NULL);
		}
	}

	if (!file) _dev.Release();
	return file;
}


bool FatFile::Seek(filepos_t new_pos)
{
	_pos = new_pos;
	return true;
}


uint FatFile::Read(void* buf, uint numbytes)
{
	uint result = 0;

	if (_pos + numbytes > _size)  numbytes = _size - _pos;

	uint8_t* bufp = (uint8_t*)buf;

	while (numbytes) {
		if (!BufferSector(FileposToSector(_pos))) break;

		const uint tocopy = min<uint>(512 - (_pos & 511), numbytes);
		assert(tocopy);

		memcpy(bufp, _sector + (_pos & 511), tocopy);

		_pos += tocopy;
		bufp += tocopy;
		result += tocopy;
		numbytes -= tocopy;
	}

	return result;
}


uint FatFile::Read(Deque<uint8_t>& buf, uint numbytes)
{
	if (_pos + numbytes > _size)  numbytes = _size - _pos;

	return Read(buf + buf.Grow(numbytes), numbytes);
}


uint FatFile::Write(const void* buf, uint numbytes)
{
	return 0;
}


void FatFile::Close()
{
	_fat._dev.Release();
	delete this;
}


bool FatFile::BufferSector(uint32_t sector)
{
	if (_buf_sec == NOT_FOUND || _buf_sec != sector) {
		if (_fat.LoadDataSector(_sector, sector)) {
			_buf_sec = sector;
		} else {
			// I/O error
			_buf_sec = NOT_FOUND;
			return false;
		}
	}
	return true;
}
