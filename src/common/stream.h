/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2022 Deborah Servilla
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMMON_STREAM_H
#define COMMON_STREAM_H

#include <sys/types.h> // for off_t and ssize_t. not portable...

#include <cstdint>
#include <istream>
#include <memory>
#include <vector>

namespace Common {

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
	void writePascalString(const std::string &value);
};

}

#endif
