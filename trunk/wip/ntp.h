#ifndef __NTP_H__
#define __NTP_H__

struct NOVTABLE NtpTime {
	uint32_t sec;
	uint32_t frac;
};


struct NOVTABLE Ntph {
	uint8_t leap:2;				// Imminent leap indicator
	uint8_t ver:3;				// Version number (3)
	enum {
		MODE_CLIENT=3,
		MODE_SERVER=4
	};
	uint8_t mode:3;
	uint8_t stratum;
	uint8_t poll;
	int8_t precision;			// Time precision (2^x, -20 for 1 usec)

	int32_t root_delay;
	int32_t root_disp;			// Root dispersion
	in_addr_t ref_id; // Reference clock identifier (IPv4 addr for st>=2)

	NtpTime reftime;			// Reference timestamp
	NtpTime origtime;			// Originator timestamp
	NtpTime recvtime;			// Receive timestamp
	NtpTime xmittime;			// Transmit timestamp
};

#endif // __NTP_H__
