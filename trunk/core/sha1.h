// public api for steve reid's public domain SHA-1 implementation
// this file is in the public domain

#ifndef __SHA1_H__
#define __SHA1_H__


class Sha1 {
	uint32_t _state[5];
	uint32_t _count[2];
	uint8_t  _buffer[64];
public:
	enum { DIGEST_SIZE = 20 };

	Sha1() { Init(); }
	void Init();
	void Update(const uint8_t* data, const size_t len);
	void Final(uint8_t digest[DIGEST_SIZE]);

	// Convenience function to hash a block
	static void Hash(const void* block, uint len, uint8_t digest[DIGEST_SIZE]);
private:
	static void Transform(uint32_t state[5], const uint8_t buffer[64]);

	void Print(const uchar *msg) {
		DMSG("%s (%d,%d) %x %x %x %x %x",
			 msg, _count[0], _count[1], _state[0], _state[1],
			 _state[2], _state[3], _state[4]);
	}
};
#endif // __SHA1_H__
