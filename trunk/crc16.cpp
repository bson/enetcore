#include "enetkit.h"
#include "crc16.h"


void Crc16::Update(uint8_t d)
{
    for (uint i = 0; i < 8; ++i) {
        const uint16_t b = (d & (0x01 << i)) ? 0x0001 : 0x0000;
        const uint16_t carry = (_crc & 0x0001) ^ b;
        _crc >>= 1;
        if (carry) _crc ^= 0x8408;
    }
}


void Crc16::Update(const uint8_t* block, uint len)
{
	while (len--)  Update(*block++);
}


// * static
uint16_t Crc16::Checksum(const void* block, uint len)
{
	Crc16 crc;
	crc.Update((const uint8_t*)block, len);
	return crc.GetValue();
}
