#ifndef __ERROR_H__
#define __ERROR_H__

enum {
	NO_ERROR = 0,
	ERR_NO_ROUTE = 1,
	ERR_CONN_REFUSED = 2,
	ERR_NO_DATA = 3,			// Nothing to recv
	ERR_NO_SPACE = 4			// No space in buffer for send
};

#endif // __ERROR_H__
