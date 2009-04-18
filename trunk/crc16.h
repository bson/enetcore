#ifndef __CRC16_H__
#define __CRC16_H__


// CRC-16 CCITT

class Crc16 {
	uint16_t _crc;
public:
	Crc16() : _crc(0x8408) { }
	void Update(uint8_t byte);
	void Update(const uint8_t* block, uint len);
	uint16_t GetValue() const { return _crc; }

	static uint16_t Checksum(const void* block, uint len);
};

#endif // __CRC16_H__
