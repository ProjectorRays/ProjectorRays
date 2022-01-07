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

#include <boost/format.hpp>
#include <boost/endian/conversion.hpp>
#include <zlib.h>

#include "common/log.h"
#include "common/stream.h"

namespace Common {

/* BufferView */

size_t BufferView::size() const {
	return _size;
}

uint8_t *BufferView::data() const {
	return _data;
}

/* Stream */

size_t Stream::pos() const {
	return _pos;
}

off_t Stream::lseek(off_t offset, int whence) {
	switch (whence) {
	case SEEK_SET:
		_pos = offset;
		break;
	case SEEK_CUR:
		_pos += offset;
		break;
	case SEEK_END:
		_pos = _size + offset;
		break;
	default:
		return -1;
	}
	return _pos;
}

void Stream::seek(size_t pos) {
	_pos = pos;
}

void Stream::skip(off_t offset) {
	_pos += offset;
}

bool Stream::eof() const {
	return  _pos >= _size;
}

bool Stream::pastEOF() const {
	return  _pos > _size;
}

/* ReadStream */

BufferView ReadStream::readByteView(size_t len) {
	BufferView res(_data + _pos, len);
	_pos += len;
	return res;
}

ssize_t ReadStream::readUpToBytes(size_t len, uint8_t *dest) {
	if (eof())
		return 0;

	if (_pos + len > _size)
		len = _size - _pos;

	memcpy(dest, &_data[_pos], len);

	_pos += len;
	return len;
}

ssize_t ReadStream::readZlibBytes(size_t len, uint8_t *dest, size_t destLen) {
	size_t p = _pos;
	_pos += len;
	if (pastEOF()) {
		throw std::runtime_error("ReadStream::readZlibBytes: Read past end of stream!");
	}

	unsigned long outLen = destLen;
	int ret = uncompress(dest, &outLen, &_data[p], len);
	if (ret != Z_OK) {
		Common::warning(boost::format("zlib decompression error %d!") % ret);
		return -1;
	}

	return outLen;
}

uint8_t ReadStream::readUint8() {
	size_t p = _pos;
	_pos += 1;
	if (pastEOF()) {
		throw std::runtime_error("ReadStream::readUint8: Read past end of stream!");
	}

	return _data[p];
}

int8_t ReadStream::readInt8() {
	return (int8_t)readUint8();
}

uint16_t ReadStream::readUint16() {
	size_t p = _pos;
	_pos += 2;
	if (pastEOF()) {
		throw std::runtime_error("ReadStream::readUint16: Read past end of stream!");
	}

	return endianness
		? boost::endian::load_little_u16(&_data[p])
		: boost::endian::load_big_u16(&_data[p]);
}

int16_t ReadStream::readInt16() {
	return (int16_t)readUint16();
}

uint32_t ReadStream::readUint32() {
	size_t p = _pos;
	_pos += 4;
	if (pastEOF()) {
		throw std::runtime_error("ReadStream::readUint32: Read past end of stream!");
	}

	return endianness
		? boost::endian::load_little_u32(&_data[p])
		: boost::endian::load_big_u32(&_data[p]);
}

int32_t ReadStream::readInt32() {
	return (int32_t)readUint32();
}

double ReadStream::readDouble() {
	size_t p = _pos;
	_pos += 4;
	if (pastEOF()) {
		throw std::runtime_error("ReadStream::readDouble: Read past end of stream!");
	}

	uint64_t f64bin = endianness
		? boost::endian::load_little_u64(&_data[p])
		: boost::endian::load_big_u64(&_data[p]);

	return *(double *)(&f64bin);
}

double ReadStream::readAppleFloat80() {
	// Adapted from @moralrecordings' code
	// from engines/director/lingo/lingo-bytecode.cpp in ScummVM

	// Floats are stored as an "80 bit IEEE Standard 754 floating
	// point number (Standard Apple Numeric Environment [SANE] data type
	// Extended).

	size_t p = _pos;
	_pos += 10;
	if (pastEOF()) {
		throw std::runtime_error("ReadStream::readAppleFloat80: Read past end of stream!");
	}

	uint16_t exponent = boost::endian::load_big_u16(&_data[p]);
	uint64_t f64sign = (uint64_t)(exponent & 0x8000) << 48;
	exponent &= 0x7fff;
	uint64_t fraction = boost::endian::load_big_u64(&_data[p + 2]);
	fraction &= 0x7fffffffffffffffULL;
	uint64_t f64exp = 0;
	if (exponent == 0) {
		f64exp = 0;
	} else if (exponent == 0x7fff) {
		f64exp = 0x7ff;
	} else {
		int32_t normexp = (int32_t)exponent - 0x3fff;
		if ((-0x3fe > normexp) || (normexp >= 0x3ff)) {
			throw std::runtime_error("Constant float exponent too big for a double");
		}
		f64exp = (uint64_t)(normexp + 0x3ff);
	}
	f64exp <<= 52;
	uint64_t f64fract = fraction >> 11;
	uint64_t f64bin = f64sign | f64exp | f64fract;
	return *(double *)(&f64bin);
}

uint32_t ReadStream::readVarInt() {
	uint32_t val = 0;
	uint8_t b;
	do {
		b = readUint8();
		val = (val << 7) | (b & 0x7f); // The 7 least significant bits are appended to the result
	} while (b >> 7); // If the most significant bit is 1, there's another byte after
	return val;
}

std::string ReadStream::readString(size_t len) {
	size_t p = _pos;
	_pos += len;
	if (pastEOF()) {
		throw std::runtime_error("ReadStream::readString: Read past end of stream!");
	}

	char *str = new char[len + 1];
	memcpy(str, &_data[p], len);
	str[len] = '\0';
	std::string res(str);
	delete[] str;
	return res;
}

std::string ReadStream::readCString() {
	std::string res;
	char ch = readInt8();
	while (ch) {
		res += ch;
		ch = readInt8();
	}
	return res;
}

std::string ReadStream::readPascalString() {
	uint8_t len = readUint8();
	return readString(len);
}

/* WriteStream */

size_t WriteStream::writeBytes(const void *dataPtr, size_t dataSize) {
	size_t p = _pos;
	_pos += dataSize;
	if (pastEOF()) {
		throw std::runtime_error("WriteStream::writeBytes: Write past end of stream!");
	}

	memcpy(&_data[p], dataPtr, dataSize);
	return dataSize;
}

size_t WriteStream::writeBytes(const Common::BufferView &view) {
	return writeBytes(view.data(), view.size());
}

void WriteStream::writeUint8(uint8_t value) {
	size_t p = _pos;
	_pos += 1;
	if (pastEOF()) {
		throw std::runtime_error("WriteStream::writeUint8: Write past end of stream!");
	}

	_data[p] = value;
}

void WriteStream::writeInt8(int8_t value) {
	writeUint8((uint8_t)value);
}

void WriteStream::writeUint16(uint16_t value) {
	size_t p = _pos;
	_pos += 2;
	if (pastEOF()) {
		throw std::runtime_error("WriteStream::writeUint16: Write past end of stream!");
	}

	if (endianness)
		boost::endian::store_little_u16(&_data[p], value);
	else
		boost::endian::store_big_u16(&_data[p], value);
}

void WriteStream::writeInt16(int16_t value) {
	writeUint16((uint16_t)value);
}

void WriteStream::writeUint32(uint32_t value) {
	size_t p = _pos;
	_pos += 4;
	if (pastEOF()) {
		throw std::runtime_error("WriteStream::writeUint32: Write past end of stream!");
	}

	if (endianness)
		boost::endian::store_little_u32(&_data[p], value);
	else
		boost::endian::store_big_u32(&_data[p], value);
}

void WriteStream::writeInt32(int32_t value) {
	writeUint32((uint32_t)value);
}

void WriteStream::writeDouble(double value) {
	size_t p = _pos;
	_pos += 8;
	if (pastEOF()) {
		throw std::runtime_error("WriteStream::writeDouble: Write past end of stream!");
	}

	if (endianness)
		boost::endian::store_little_u64(&_data[p], *(uint64_t *)(&value));
	else
		boost::endian::store_big_u64(&_data[p], *(uint64_t *)(&value));
}

void WriteStream::writeString(const std::string &value) {
	writeBytes(value.c_str(), value.size());
}

void WriteStream::writePascalString(const std::string &value) {
	writeUint8(value.size());
	writeString(value);
}

}
