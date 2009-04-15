#ifndef __FAT_H__
#define __FAT_H__

#include "blockdev.h"


// PC partition entry
struct NOVTABLE PartEnt {
	uint8_t boot_flag;
	uint8_t chs_begin[3];
	uint8_t type_code;
	uint8_t chs_end[3];
	uint32_t lba_begin;
	uint32_t num_sect;
};


class Fat {
	BlockDev& _dev;
	uint32_t _lba0;				// First LBA of partition
	uint32_t _size;				// Partition size in sectors
	uint32_t _fat_num_sect;		// Sectors per FAT
	uint32_t _root_dir_clus;	// Root directory cluster
	uint16_t _resv_clus;		// Rserved clusters
	uint8_t _sec_per_clus;		// Sectors per cluster
	uint8_t _clus_bits;			// Log2(_sec_per_clus)
	uint32_t _fat_sector;		// First sector of FAT
	uint32_t _cluster0;			// Sector of first cluster (following FAT).
	uint16_t _max_root;			// FAT16: Max root dir size
	bool _fat32:1;				// Volume is FAT32 (otherwise FAT16)
	bool _rw:1;					// Read-write mount

	// Volume ID offsets
	enum {
		VID_Bytes_Per_Sec = 0xb,	// 16 bits (we expect 512)
		VID_Sect_Per_Clus = 0xd,	// 8 bits
		VID_Res_Sects = 0xe,		// Reserved sectors, 16 bits
		VID_Num_FATs = 0x10,		// Number of FATs (always 2), 8 bits
		VID_Sect_Per_FAT = 0x24,	// Sectors per FAT, 32 bits
		VID_Root_Dir_Clus = 0x2c,	// Root dir cluster, 32 bits (usually 2)
		VID_Max_Root_Dir = 0x11		// FAT16: max # of entries in root dir
	};

public:
	Fat(BlockDev& dev);

	// Mount first partition on device
	bool Mount(uint partnum, bool rw);
	
	
private:
	// Simple numeric conversions
	uint32_t ClusterToSector(uint32_t cluster) const { return cluster << _clus_bits; }
	uint32_t SectorToCluster(uint32_t sector) const { return sector >> _clus_bits; }
	bool IsValidCluster(uint32_t cluster) const { return ClusterToSector(cluster) < _size; }

	// Find LBA for first sector in data cluster
	uint32_t DataClusterToLBA(uint32_t cluster) const {
		return ClusterToSector(cluster) + _cluster0;
	}
};

#endif // __FAT_H__
