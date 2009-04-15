#include "enetkit.h"
#include "fat.h"


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

	_lba0 = part[partnum].lba_begin;
	_size = part[partnum].num_sect;

	if (!_lba0 || !_size) goto failed;

	_fat32 = false;

	type = part[partnum].type_code;

	if (type == 0xb || type == 0xc) _fat32 = true;
	else if (part[partnum].type_code != 6)  goto failed;

	// Read volume ID
	if (!_dev.ReadSector(_lba0, sector)) goto failed;
	if (*(uint16_t*)(sector + 510) != LE16(0xaa55)) goto failed;

	memcpy(&_fat_num_sect, sector + VID_Sect_Per_FAT, sizeof _fat_num_sect);
	_fat_num_sect = LE32(_fat_num_sect);

	memcpy(&_root_dir_clus, sector + VID_Root_Dir_Clus, sizeof _root_dir_clus);
	_root_dir_clus = LE32(_root_dir_clus);

	memcpy(&_resv_clus, sector + VID_Res_Sects, sizeof _resv_clus);
	_resv_clus = LE16(_resv_clus);

	_sec_per_clus = sector[VID_Sect_Per_Clus];
	_clus_bits = Util::ffs(_sec_per_clus);

	if (!_fat_num_sect || _sec_per_clus != (1 << _clus_bits) ||
		!IsValidCluster(_root_dir_clus) ||
		sector[VID_Num_FATs] != 2)
		goto failed;

	_fat_sector = _lba0 + _resv_clus;
	_cluster0 = _fat_sector + 2 * _fat_num_sect;

	// FAT16: advance _cluster0 past root dir and fill in _root_dir_clus
	_max_root = 0;
	if (!_fat32) {
		memcpy(&_max_root, sector + VID_Max_Root_Dir, 2);
		_max_root = LE16(_max_root);
		_root_dir_clus = _cluster0;
		_cluster0 += _max_root / 32;
	}

	ok = true;
		
failed:
	xfree(sector);
	return ok;
}
