#include "enetkit.h"
#include "dns.h"


uint16_t Dns::_id = 0;
String Dns::_domain;


// * static
void Dns::CreateLookupQuery(Deque<uint8_t>& buf, const String& host)
{
	String fqdn(host);

	if (fqdn.FindFirst('.') == NOT_FOUND) {
		fqdn += STR(".");
		fqdn += GetDomain();
	}

	Dnsh& dnsh = *(Dnsh*)(buf + buf.Grow(sizeof (Dnsh)));

	memset(&dnsh, 0, sizeof dnsh);

	// dnsh.opcode = OPCODE_QUERY;

	dnsh.id = Htons(++_id);
	dnsh.rd = true;
	dnsh.questions = Htons(1);

	const uint query_len = 4 + fqdn.Size() + 1;

	uint8_t* query = buf + buf.Grow(query_len);

	((uint32_t*)(query + query_len))[-1] = 0; // Zero last 4 bytes

	uint pos = 0;				// Current char in fqdn
	uint part_pos = 0;			// First char in fqdn part
	uint part_query = 0;		// Current char in query part
	uint query_pos = 0;			// Current pos in query

	const uint host_len = fqdn.Size();

	while (pos < host_len) {
		if (fqdn[pos] == '.')  {
			const uint part_len = pos - part_pos;
			query[part_query] = part_len;
			part_query += part_len + 1;
			part_pos = ++pos;
			++query_pos;
		} else {
			query[++query_pos] = fqdn[pos++];
		}
	}

	const uint16_t type_class[2] = { Htons(TYPE_A), Htons(CLASS_IN) };

	memcpy(buf + buf.Grow(4), type_class, 4);
}
