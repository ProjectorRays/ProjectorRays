#ifndef SUBCHUNK_H
#define SUBCHUNK_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "util.h"

namespace ProjectorRays {

struct Datum;
class ReadStream;

struct CastListEntry {
    std::string name;
    std::string filePath;
    uint16_t preloadSettings;
    uint16_t minMember;
    uint16_t maxMember;
    int32_t id;
};

struct MemoryMapEntry {
    uint32_t fourCC;
    int32_t len;
    int32_t offset;
    uint16_t flags;
    int16_t unknown0;
    int32_t next;

    void read(ReadStream &stream);
};

struct ScriptContextMapEntry {
    int32_t unknown0;
    int32_t sectionID;
    uint16_t unknown1;
    uint16_t unknown2;

    void read(ReadStream &stream);
};

enum LiteralType {
    kLiteralString  = 1,
    kLiteralInt     = 4,
    kLiteralFloat   = 9
};

struct KeyTableEntry {
    int32_t sectionID;
    int32_t castID;
    uint32_t fourCC;

    void read(ReadStream &stream);
};

struct LiteralStore {
    LiteralType type;
    uint32_t offset;
    std::shared_ptr<Datum> value;

    void readRecord(ReadStream &stream);
    void readData(ReadStream &stream, uint32_t startOffset);
};

struct Rectangle {
    uint16_t top;
    uint16_t left;
    uint16_t bottom;
    uint16_t right;

    void read(ReadStream &stream);
};

}

#endif
