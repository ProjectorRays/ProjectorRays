#ifndef SUBCHUNK_H
#define SUBCHUNK_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "util.h"

namespace ProjectorRays {

struct Datum;
struct ReadStream;

struct CastKeyEntry {
    uint32_t sectionID;
    uint32_t castID;
    uint32_t fourCC;

    void read(ReadStream &stream);
};

struct CastDataEntry {
    std::string name;
    std::string filePath;
    uint16_t preloadSettings;
    uint16_t storageType;
    uint16_t membersCount;
    uint32_t id;

    void read(ReadStream &stream);
};

struct MemoryMapEntry {
    uint32_t fourCC;
    uint32_t len;
    uint32_t offset;
    int16_t padding;
    int16_t unknown0;
    int32_t link;

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
