#ifndef __ETHERNET_H__
#define __ETHERNET_H__

namespace BufferPool {
	void Initialize(uint num);
	IOBuffer* Alloc();
	void FreeBuffer(IOBuffer* buf);
}

#endif // __ETHERNET_H__
