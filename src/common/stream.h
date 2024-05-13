/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_STREAM_H
#define COMMON_STREAM_H

#include <sys/types.h> // for off_t and ssize_t. not portable...

#include <cstdint>
#include <istream>
#include <memory>
#include <vector>

namespace Common {

class String;

enum Endianness {
	kBigEndian = 0,
	kLittleEndian = 1
};

/* BufferView */

class BufferView {
protected:
	uint8_t *_data;
	size_t _size;

public:
	BufferView()
		: _data(nullptr), _size(0) {}

	BufferView(uint8_t *d, size_t s)
		: _data(d), _size(s) {}

	size_t size() const;
	uint8_t *data() const;
};

/* Stream */

class Stream : public BufferView {
protected:
	size_t _pos;

public:
	Endianness endianness;

	Stream(uint8_t *d, size_t s, Endianness e = kBigEndian, size_t p = 0)
		: BufferView(d, s), _pos(p), endianness(e) {}

	Stream(const BufferView &view, Endianness e = kBigEndian, size_t p = 0)
		: BufferView(view.data(), view.size()), _pos(p), endianness(e) {}

	size_t pos() const;
	off_t lseek(off_t offset, int whence);
	void seek(size_t pos);
	void skip(off_t offset);
	bool eof() const;
	bool pastEOF() const;
};

/* ReadStream */

class ReadStream : public Stream {
public:
	ReadStream(uint8_t *d, size_t s, Endianness e = kBigEndian, size_t p = 0)
		: Stream(d, s, e, p) {}

	ReadStream(const BufferView &view, Endianness e = kBigEndian, size_t p = 0)
		: Stream(view, e, p) {}

	BufferView readByteView(size_t len);
	ssize_t readUpToBytes(size_t len, uint8_t *dest);
	ssize_t readZlibBytes(size_t len, uint8_t *dest, size_t destLen);
	uint8_t readUint8();
	int8_t readInt8();
	uint16_t readUint16();
	int16_t readInt16();
	uint32_t readUint32();
	int32_t readInt32();
	double readDouble();
	double readAppleFloat80();
	uint32_t readVarInt();
	std::string readString(size_t len);
	std::string readCString();
	std::string readPascalString();
};

/* WriteStream */

class WriteStream : public Stream {
public:
	WriteStream(uint8_t *d, size_t s, Endianness e = kBigEndian, size_t p = 0)
		: Stream(d, s, e, p) {}

	WriteStream(const BufferView &view, Endianness e = kBigEndian, size_t p = 0)
		: Stream(view, e, p) {}

	size_t writeBytes(const void *dataPtr, size_t dataSize);
	size_t writeBytes(const Common::BufferView &view);
	void writeUint8(uint8_t value);
	void writeInt8(int8_t value);
	void writeUint16(uint16_t value);
	void writeInt16(int16_t value);
	void writeUint32(uint32_t value);
	void writeInt32(int32_t value);
	void writeDouble(double value);
	void writeString(const std::string &value);
	void writeString(const Common::String &value);
	void writePascalString(const std::string &value);
	void writePascalString(const Common::String &value);
};

} // namespace Common

#endif // COMMON_STREAM_H
