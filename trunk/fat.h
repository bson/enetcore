#ifndef __FAT_H__
#define __FAT_H__

#include "file.h"
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


// FAT directory entry
struct NOVTABLE FatDirEnt {
	uint8_t name[11];			// 8.3 short file name
	uint8_t attrib;				// File attribute
	uint8_t pad[8];
	uint16_t clushi;			// Cluster, high 16 bits
	uint8_t pad2[4];
	uint16_t cluslo;			// Cluster, low 16 bits
	uint32_t size;				// File size in bytes

	INLINE_ALWAYS uint32_t GetCluster() const { return (LE16(clushi) << 16) + LE16(cluslo); }
	INLINE_ALWAYS bool IsUsed() const { return name[0] != 0xe5; }
	INLINE_ALWAYS bool IsLFN() const { return (attrib & 0xf) == 0xf; }
	INLINE_ALWAYS bool IsVolume() const { return (attrib & 8) != 0; }
	INLINE_ALWAYS bool IsDir() const { return (attrib & 16) != 0; }
	INLINE_ALWAYS bool IsRO() const { return (attrib & 1) != 0; }
};


class FatFile;

class Fat {
	// XXX we don't really need all this state
	BlockDev& _dev;
	uint32_t _lba0;				// First LBA of partition
	uint32_t _size;				// Partition size in sectors
	uint32_t _fat_num_sect;		// Sectors per FAT
	uint32_t _root_dir_clus;	// Root directory cluster (FAT16: root dir sector)
	uint16_t _resv_sect;		// Reserved sectors
	uint8_t _sec_per_clus;		// Sectors per cluster
	uint8_t _clus_bits;			// Log2(_sec_per_clus)
	uint32_t _fat_sector;		// First sector of FAT
	uint32_t _cluster0;			// Sector of first cluster (following FAT).
	uint16_t _max_root;			// FAT16: Max root dir size in bytes
	bool _fat32:1;				// Volume is FAT32 (otherwise FAT16)
	bool _rw:1;					// Read-write mount

	// Volume ID offsets
	enum {
		VID_Bytes_Per_Sec = 0xb,	// 16 bits (we expect 512)
		VID_Sect_Per_Clus = 0xd,	// 8 bits
		VID_Res_Sects = 0xe,		// Reserved sectors, 16 bits
		VID_Num_FATs = 0x10,		// Number of FATs (always 2), 8 bits
		VID_Sect_Per_FAT = 0x24,	// Sectors per FAT, 32 bits
		VID16_Sect_Per_FAT = 0x16,	// Sectors per FAT16, 16 bits
		VID_Root_Dir_Clus = 0x2c,	// Root dir cluster, 32 bits (usually 2)
		VID_Max_Root_Dir = 0x11, // FAT16: max # of entries in root dir
		VID16_Volume_Name = 0x2b,	// FAT16: volume name
		VID_Volume_Name = 0x47		// FAT32: volume name
	};

public:
	Fat(BlockDev& dev);

	// Mount specified partition
	bool Mount(uint partnum, bool rw);
	
	// Open file
	FatFile* Open(const String& path);

protected:
	friend class FatFile;

	// Simple numeric conversions
	INLINE_ALWAYS uint32_t ClusterToSector(uint32_t cluster) const {
		return cluster << _clus_bits;
	}
	INLINE_ALWAYS uint32_t SectorToCluster(uint32_t sector) const {
		return sector >> _clus_bits;
	}
	INLINE_ALWAYS bool IsValidCluster(uint32_t cluster) const {
		return ClusterToSector(cluster) < _size;
	}

	// Find LBA for first sector in data cluster
	INLINE_ALWAYS uint32_t DataClusterToLBA(uint32_t cluster) const {
		return ClusterToSector(cluster) + _cluster0;
	}

	// Test if position is in particular cluster
	INLINE_ALWAYS bool IsInCluster(uint32_t cluster, filepos_t pos) {
		return SectorToCluster(pos / 512) == cluster;
	}

	// Find directory entry, or NULL if not found
	FatDirEnt* FindFile(const Vector<uint8_t>& dir, const String& name);

	// Obtain cluster list for file
	bool GetFileClusters(Vector<uint32_t>& clusters, uint32_t cluster1);

	template <typename Cluster>
	bool GetFileClustersImpl(Vector<uint32_t>& clusters, uint32_t cluster1);

	// Load data cluster into memory, appending to buffer
	bool LoadDataCluster(Vector<uint8_t>& buffer, uint32_t cluster);
	bool LoadDataClusters(Vector<uint8_t>& buffer, Vector<uint32_t>& cluster_list);

	// Load data sector(s) into memory
	bool LoadDataSector(uint8_t* buf, uint32_t sector);
	bool LoadDataSectors(Vector<uint8_t>& buf, uint32_t sector, uint num_sectors);
};


class FatFile: public File {
protected:
	friend class Fat;

	class Fat& _fat;			// File system this file is on

	Vector<uint32_t> _clusters;	// File cluster list
	filepos_t _size;			// File size, in bytes
	filepos_t _pos;				// Current file position

	uint32_t _buf_sec;			// Sector in buffer (NOT_FOUND if none)
	uint8_t _sector[512];		// Sector buffer

	FatFile(Fat& fs) : _fat(fs), _pos(0), _buf_sec(NOT_FOUND) { }
	~FatFile() { }
public:
	uint32_t GetSize() const { return _size; }
	bool Seek(filepos_t new_pos);
	filepos_t Tell() const { return _pos; }
	uint Read(void* buf, uint numbytes);
	uint Read(Deque<uint8_t>& buf, uint numbytes);
	uint Write(const void* buf, uint numbytes);
	void Close();
private:
	bool BufferSector(uint32_t sector);

	uint32_t FileposToCluster(filepos_t pos) const {
		return _clusters[_fat.SectorToCluster(pos / 512)] - 2;
	}

	uint32_t FileposToSector(filepos_t pos) const {
		return _fat.ClusterToSector(FileposToCluster(pos)) +
			((pos / 512) & (_fat.ClusterToSector(1) - 1));
	}
};


extern Fat _fat;

#endif // __FAT_H__
