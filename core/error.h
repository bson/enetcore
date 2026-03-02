// Copyright (c) 2018-2026 Jan Brittenson
// See LICENSE for details.
#define __ERROR_H__

enum {
	NO_ERROR = 0,
	ERR_NO_ROUTE = 1,
	ERR_CONN_REFUSED = 2,
	ERR_NO_DATA = 3,			// Nothing to recv
	ERR_NO_SPACE = 4,			// No space in buffer for send
	ERR_BAD_OP = 5,				// Attempt to e.g. Accept on a UDP socket
	ERR_ADDR_IN_USE = 6,
	ERR_NOT_CONNECTED = 7,
	ERR_DEST_UNREACH = 8
};
