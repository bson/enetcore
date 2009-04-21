#ifndef __BLOCKDEV_H__
#define __BLOCKDEV_H__

#include "gpio.h"


class BlockDev {
public:
	virtual bool Init() = 0;
	virtual uint GetSectorSize() const = 0;
	virtual bool ReadSector(uint secnum, void* buf) = 0;
	virtual bool WriteSector(uint secnum, const void* buf) = 0;

	// Prevent media removal
	virtual void Retain() = 0;
	virtual void Release() = 0;
};

#endif // __BLOCKDEV_H__
