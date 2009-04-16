#ifndef __FILE_H__
#define __FILE_H__

typedef class File* file_t;
typedef uint32_t filepos_t;

class File {
public:
	virtual uint32_t GetSize() const = 0;
	virtual bool Seek(filepos_t new_pos) = 0;
	virtual filepos_t Tell() const = 0;
	virtual uint Read(void* buf, uint numbytes) = 0;
	virtual uint Read(Deque<uint8_t>& buf, uint numbytes) = 0;
	virtual uint Write(const void* buf, uint numbytes) = 0;
	virtual void Close() = 0;

protected:
	// Don't delete File; use Close()
	virtual ~File() { }
};

#endif // __FILE_H__
