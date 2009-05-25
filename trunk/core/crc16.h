#ifndef __CRC16_H__
#define __CRC16_H__


// CRC-16 CCITT

class Crc16 {
	uint16_t _crc;
#if 0
#ifndef GENTABLE
	const
#endif
	static uint16_t _crc_table[256];
#endif
public:
	// SD cards appear to use an initial value of 0 and use the sum, whereas
	// real CCITT CRC-16 use a value of 0xffff and use the 1's complement.
	Crc16(uint16_t initial = 0) : _crc(initial) { }		
	void Update(uint8_t byte);
	void Update(const uint8_t* block, uint len);
	uint16_t GetValue() const { return _crc; }

	static uint16_t Checksum(const void* block, uint len, uint16_t initial = 0);
#ifdef GENTABLE
	static void GenTable();
#endif
};

#endif // __CRC16_H__
