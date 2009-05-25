#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "mutex.h"


// Socket interface.  Sockets are always non-blocking by design.
// Event delivery is edge triggered.

class CoreSocket: public EventObject {
	uint16_t _evmask;			// Events that signal EO
	uint16_t _event;			// Current events
	uint8_t _error;				// Last socket error
public:
	enum {
		EVENT_READABLE = 1,		// Recv queue is non-empty (or listen socket has conn)
		EVENT_WRITEABLE = 2,	// Send queue is less than full
		EVENT_CONNECT = 4,		// Socket is connected
		EVENT_CLOSE = 8,		// Socket is closed
		EVENT_ERROR = 16		// Connection error (get reason with GetError())
	};

	CoreSocket() : _error(NO_ERROR), _evmask(0), _event(0) { }
	virtual ~CoreSocket() { }

	virtual bool Listen(uint backlog) { _error = ERR_BAD_OP; return false; }
	virtual CoreSocket* Accept(NetAddr& from) { _error = ERR_BAD_OP; return false; }
	virtual bool Bind(const NetAddr& arg) = 0;
	virtual bool Connect(const NetAddr& dest) = 0;
	virtual bool GetSockAddr(NetAddr& addr) = 0;
	virtual bool GetPeerAddr(NetAddr& addr) = 0;
	virtual uint GetRecvAvail() = 0;
	virtual uint GetSendSpace() = 0;
	virtual bool Send(const void* data, uint len) = 0;
	virtual bool SendTo(const void* data, uint len, const NetAddr& dest) {
		_error = ERR_BAD_OP; return false;
	}
	virtual bool Recv(void* data, uint& len) = 0;
	virtual bool RecvFrom(void* data, uint& len, NetAddr& sender) {
		_error = ERR_BAD_OP; return false;
	}
	virtual bool Shutdown(int dir) { _error = ERR_BAD_OP; return false; }
	virtual bool Close() = 0;

	uint GetEvent() { return _event; }
	bool SetEventMask(uint mask) { _evmask = mask; return true; }
	void AddEvent(uint event) {
		_event |= event; if (event & _evmask)  EventObject::Set();
	}
	void ClearEvent(uint event) { _event &= ~event; }
	void SetError(uint e) { _error = e; }
	uint GetError() { return _error; }
};

#endif // __SOCKET_H__
