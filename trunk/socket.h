#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "mutex.h"


// Socket interface.  Sockets are always non-blocking by design.
class CoreSocket: public EventObject {
public:
	enum {
		EVENT_READABLE = 1,		// Recv queue is non-empty
		EVENT_WRITEABLE = 2,	// Send queue is less than full
		EVENT_CONNECTED = 4,	// Socket is connected
		EVENT_CLOSE = 8			// Socket is closed
	};

	// These functions return true on success, false on failure.
	// On failure, error can be obtained with GetLastError().
	virtual bool Listen(uint backlog) = 0;
	virtual bool Bind(const NetAddr& arg) = 0;
	virtual bool Connect(const NetAddr& dest) = 0;
	virtual bool GetSockAddr(NetAddr& addr) = 0;
	virtual bool GetPeerAddr(NetAddr& addr) = 0;
	virtual bool GetEvent(uint& state) = 0;
	virtual bool SetEventMask(uint mask) = 0;
	virtual bool Send(const void* data, uint len) = 0;
	virtual bool SendTo(const void* data, uint len, const NetAddr& dest) = 0;
	virtual bool Recv(void* data, uint& len) = 0;
	virtual bool RecvFrom(void* data, uint& len, NetAddr& sender) = 0;
	virtual bool Shutdown(int dir) = 0;
	virtual bool Close() = 0;
};

#endif // __SOCKET_H__
