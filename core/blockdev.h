// Copyright (c) 2018-2021 Jan Brittenson
// See LICENSE for details.

#ifndef __BLOCKDEV_H__
#define __BLOCKDEV_H__

#include <stdint.h>

class BlockDev {
public:
    // Initialize this and underlying
    virtual int init() = 0;

    virtual int read_blocks(uint32_t lba,
                            uint32_t count,
                            void *buffer,
                            bool bypass = false) = 0;

    virtual int write_blocks(uint32_t lba,
                             uint32_t count,
                             const void *buffer,
                             bool bypass = false) = 0;

    // Flush any caches or buffers
    virtual int flush() = 0;
    
    // Return device sector size
    virtual uint32_t sector_size() const = 0;

    // Return device size in sectors.  Returns 0 if unknown.
    virtual uint32_t size() const = 0;

	virtual void retain() = 0;
	virtual void release() = 0;
};

#endif // __BLOCKDEV_H__
