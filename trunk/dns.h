#ifndef __DNS_H__
#define __DNS_H__

#include "ethernet.h"
#include "network.h"


// Base DNS header
struct NOVTABLE Dnsh {
	uint16_t id;
	bool response:1;			// Query/response

	enum { OPCODE_QUERY = 0 };

	uint8_t opcode:4;
	bool aa:1;					// Authoritative
	bool tc:1;					// Truncated
	bool rd:1;					// Recursion desired
	bool ra:1;					// Recursion available (from server)
	bool z:1;
	bool ad:1;					// Authenticated data
	bool cd:1;					// Checking disabled
	uint8_t rcode:4;			// Return code

	uint16_t questions;			// # of questions
	uint16_t rr_answers;		// RR answers
	uint16_t rr_auth;			// Authority RRs
	uint16_t rr_add;			// Additional RRs
};


class Dns {
	static uint16_t _id;		// ID counter

	enum {
		TYPE_A = 1,				// IPv4 addr
		CLASS_IN = 1			// INET4
	};
		
	static String _domain;

public:
	// Add forward A RR query for host to buf
	// The buffer head should be at the first byte to fill in
	static void CreateLookupQuery(IOBuffer& buf, const String& host);

	// Return default search domain
	INLINE_ALWAYS static const String& GetDomain() { return _domain; }
};

#endif // __DNS_H__
