#ifndef STREAM_H
#define STREAM_H

#include <cstdint>
#include <istream>
#include <memory>
#include <vector>

namespace ProjectorRays {

enum Endianness {
    kBigEndian = 0,
    kLittleEndian = 1
};

/* ReadStream */

class ReadStream {
private:
    std::shared_ptr<std::vector<uint8_t>> _buf;
    size_t _offset;
    size_t _len;
    size_t _pos;

public:
    Endianness endianness;

    ReadStream(std::shared_ptr<std::vector<uint8_t>> b, Endianness e = kBigEndian, size_t o = 0, size_t l = SIZE_MAX)
        : _buf(b), _offset(o), _len(l), _pos(0), endianness(e) {}

    size_t pos();
    size_t len();
    void seek(size_t p);
    void skip(size_t p);
    bool eof();
    bool pastEOF();

    std::uint8_t *getData();
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
    std::string readPascalString();
};

}

#endif
