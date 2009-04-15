#ifndef __BLOCKDEV_H__
#define __BLOCKDEV_H__


class BlockDev {
public:
	virtual void Init() = 0;
	virtual uint GetSectorSize() const = 0;
	virtual bool ReadSector(uint secnum, void* buf) = 0;
	virtual bool WriteSector(uint secnum, const void* buf) = 0;
};

#endif // __BLOCKDEV_H__
