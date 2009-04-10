#ifndef __ERROR_H__
#define __ERROR_H__

enum {
	NO_ERROR = 0,
	ERR_NO_ROUTE = 1,
	ERR_CONN_REFUSED = 2,
	ERR_TIMEOUT = 3
};


inline int GetLastError() { return Self().GetTLS()._last_error; }
inline void SetLastError(int e) { Self().GetTLS()._last_error = e; }

#endif // __ERROR_H__
