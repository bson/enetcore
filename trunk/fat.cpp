#include "enetkit.h"
#include "fat.h"
#include "util.h"


Fat::Fat(BlockDev& dev) :
	_dev(dev)
{
}


bool Fat::Mount(uint partnum, bool rw)
{
	_rw = rw;

	uint8_t* sector = (uint8_t*)xmalloc(514);
	const PartEnt* part = (PartEnt*)(sector + 448);
	bool ok = false;
	uint8_t type;

	_dev.Init();

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
	return ok;
}


FatDirEnt* Fat::FindFile(const Vector<uint8_t>& dir, const String& name)
{
	if (dir.Empty()) return NULL;

	const uint namelen = name.Size();

	uint8_t namebuf[11];
	uint dotpos = name.FindLast((uchar)'.');
	if (dotpos == NOT_FOUND)  dotpos = namelen;
	const uint namepart = min<uint>(dotpos, 8);
	memset(namebuf, ' ', sizeof namebuf);
	memcpy(namebuf, name.CStr(), namepart);
	memcpy(namebuf + 8, name.CStr() + dotpos + 1, min<uint>(namelen - dotpos, 3));

	for (uint i = 0; i < 11; ++i)
		namebuf[i] = Util::ToUpper(namebuf[i]);
		
	for (FatDirEnt* d = (FatDirEnt*)&dir.Front(); d < (FatDirEnt*)&dir.Back(); ++d) {
		if (!d->IsUsed()) continue;
		// XXX we really want to check against LFN here
		if (d->IsLFN()) continue;
		if (d->IsVolume()) continue;

		for (uint i = 0; ; ++i) {
			if (namebuf[i] != d->name[i])  break;
			if (i == 10)  return d;
		}
	}

	return NULL;
}


template <typename Cluster>
bool Fat::GetFileClustersImpl(Vector<uint32_t>& clusters, uint32_t cluster1)
{
	uint32_t fatsec = NOT_FOUND; // FAT Sector we currently have loaded
	uint8_t* fat = (uint8_t*)xmalloc(512); // FAT Sector buffer

	bool success = false;

	enum { NUM_FAT_PER_SEC = 512 / sizeof (Cluster) };

	Cluster clus = cluster1;

	while (clus && clus < (Cluster)0xfffffff0) {
		const uint32_t next_fatsec = clus / NUM_FAT_PER_SEC;
		if (next_fatsec >= _fat_num_sect) goto done;

		clusters.PushBack((uint32_t)clus);

		if (fatsec == NOT_FOUND || fatsec != next_fatsec) {
			// Need different FAT sector - load it
			if (!_dev.ReadSector(_fat_sector + next_fatsec, fat)) goto done;
			fatsec = next_fatsec;
		}

		clus = ((const Cluster*)fat)[clus & (NUM_FAT_PER_SEC - 1)];
	}

	// If we ran into anything other than an end-of-chain marker we have
	// a corrupt FAT.
	success = clus >= (Cluster)0xfffffff8;

done:
	if (!success)  console("Corrupt FAT/DIR entry for cluster %x", (uint)clus);

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
	uint32_t sector1 = ClusterToSector(cluster);
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


FatFile* Fat::Open(const String& path)
{
	Vector<String*> pathlist;
	path.Split(pathlist, STR("/"));

	Vector<uint8_t> dir;
	Vector<uint32_t> dir_clusters;

	uint32_t walk = _root_dir_clus;

	dir.Reserve(512 * ClusterToSector(1));

	// Walk the path chain
	FatDirEnt* dirent = NULL;

	FatFile* file = NULL;
	uint32_t file_size = 0;

	for (uint i = 0; i < pathlist.Size(); ++i) {
		// Load directory contents
		dir_clusters.SetSize(0);
		dir.SetSize(0);

		if (!i && !_fat32) {
			// FAT16: root dir is sector #
			if (!LoadDataSectors(dir, walk, _max_root * 32 / 512)) goto done;
		} else {
			if (!GetFileClusters(dir_clusters, walk)) goto done;
			if (!LoadDataClusters(dir, dir_clusters)) goto done;
		}

		dirent = FindFile(dir, *pathlist[i]);
		if (!dirent) goto done;

		// Check if attempting to open a dir, or descend into a file
		if (dirent->IsDir() == (i == pathlist.Size() - 1)) goto done;

		// Descend, but remember file size
		file_size = LE32(dirent->size);
		walk = dirent->GetCluster();
	}

	file = new FatFile(*this);
	file->_size = file_size;
	file->_clusters.Reserve(SectorToCluster((file_size + 511) / 512) + 1);

	if (!GetFileClusters(file->_clusters, walk))
		delete exch<FatFile*>(file, NULL);

done:
	pathlist.DeleteObjects();
	return file;
}


bool FatFile::Seek(filepos_t new_pos)
{
	_pos = new_pos;
}


uint FatFile::Read(void* buf, uint numbytes)
{
	uint result = 0;

	if (_pos + numbytes > _size)  numbytes = _size - _pos;

	while (numbytes) {
		if (!BufferSector(FileposToSector(_pos))) break;

		const uint tocopy = min<uint>(512 - (_pos & 511), numbytes);
		assert(tocopy);

		memcpy(buf, _sector + (_pos & 511), tocopy);

		_pos += tocopy;
		(uint8_t*&)buf += tocopy;
		result += tocopy;
		numbytes -= tocopy;
	}

	return result;
}


uint FatFile::Read(Deque<uint8_t>& buf, uint numbytes)
{
	uint result = 0;

	if (_pos + numbytes > _size)  numbytes = _size - _pos;

	while (numbytes) {
		if (!BufferSector(FileposToSector(_pos)))  break;

		const uint tocopy = min<uint>(512 - (_pos & 511), numbytes);
		assert(tocopy);

		buf.PushBack(_sector + (_pos & 511), tocopy);

		_pos += tocopy;
		result += tocopy;
		numbytes -= tocopy;
	}

	return result;
}


uint FatFile::Write(const void* buf, uint numbytes)
{
	return 0;
}


void FatFile::Close()
{
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
