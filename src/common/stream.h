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

#include <cstdint>
#include <istream>
#include <memory>
#include <vector>

namespace Common {

enum Endianness {
	kBigEndian = 0,
	kLittleEndian = 1
};

/* Stream */

class Stream {
protected:
	std::shared_ptr<std::vector<uint8_t>> _buf;
	size_t _offset;
	size_t _len;
	size_t _pos;

public:
	Endianness endianness;

	Stream(std::shared_ptr<std::vector<uint8_t>> b, Endianness e = kBigEndian, size_t o = 0, size_t l = SIZE_MAX)
		: _buf(b), _offset(o), _len(l), _pos(0), endianness(e) {}

	size_t pos();
	size_t len();
	void seek(size_t p);
	void skip(size_t p);
	bool eof();
	bool pastEOF();

	std::uint8_t *getData();
};

/* ReadStream */

class ReadStream : public Stream {
public:
	ReadStream(std::shared_ptr<std::vector<uint8_t>> b, Endianness e = kBigEndian, size_t o = 0, size_t l = SIZE_MAX)
		: Stream(b, e, o, l) {}

	std::shared_ptr<std::vector<uint8_t>> copyBytes(size_t len);

	std::unique_ptr<ReadStream> readBytes(size_t len);
	std::unique_ptr<ReadStream> readZlibBytes(unsigned long len, unsigned long *outLen);
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
	WriteStream(std::shared_ptr<std::vector<uint8_t>> b, Endianness e = kBigEndian, size_t o = 0, size_t l = SIZE_MAX)
		: Stream(b, e, o, l) {}

	size_t writeBytes(const void *dataPtr, size_t dataSize);
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
