#ifndef CHUNK_H
#define CHUNK_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "subchunk.h" // FIXME: get rid of header dependency
#include "util.h"

namespace ProjectorRays {

struct Handler;
struct LiteralStore;
struct Movie;

struct CastInfoChunk;
struct ScriptContextChunk;
struct ScriptNamesChunk;

struct Chunk {
    Movie *movie;

    Chunk(Movie *m) : movie(m) {}
    virtual ~Chunk() = default;
    virtual void read(ReadStream &stream) {}
};

struct ListChunk : Chunk {
    uint32_t dataOffset;
    uint16_t offsetTableLen;
    std::vector<uint32_t> offsetTable;
    uint32_t finalDataLen;
    uint32_t listOffset;

    ListChunk(Movie *m) : Chunk(m) {}
    virtual void read(ReadStream &stream);
    void readOffsetTable(ReadStream &stream);
    std::unique_ptr<ReadStream> readBytes(ReadStream &stream, uint16_t index);
    std::string readCString(ReadStream &stream, uint16_t index);
    std::string readPascalString(ReadStream &stream, uint16_t index);
    uint16_t readUint16(ReadStream &stream, uint16_t index);
    uint32_t readUint32(ReadStream &stream, uint16_t index);
};

struct CastAssociationsChunk : Chunk {
    std::vector<uint32_t> entries;

    CastAssociationsChunk(Movie *m) : Chunk(m) {}
    virtual ~CastAssociationsChunk() = default;
    virtual void read(ReadStream &stream);
};

struct CastKeyChunk : Chunk {
    uint16_t unknown0;
    uint16_t unknown1;
    uint32_t entryCount;
    uint32_t unknown2;
    std::vector<CastKeyEntry> entries;

    CastKeyChunk(Movie *m) : Chunk(m) {}
    virtual ~CastKeyChunk() = default;
    virtual void read(ReadStream &stream);
};

struct CastListChunk : ListChunk {
    uint16_t unk0;
    uint16_t castCount;
    uint16_t itemsPerCast;
    uint16_t unk1;
    std::vector<CastListEntry> entries;

    CastListChunk(Movie *m) : ListChunk(m) {}
    virtual ~CastListChunk() = default;
    virtual void read(ReadStream &stream);
};

enum CastMemberType {
    kBitmapCast = 0x01,
    kFilmLoopCast = 0x02,
    kPaletteCast = 0x04,
    kAudioCast = 0x06,
    kButtonCast = 0x07,
    kShapeCast = 0x08,
    kScriptCast = 0x0b,
    kXtraCast = 0x0f
};

struct CastMemberChunk : Chunk {
    uint32_t type;
    uint32_t infoLen;
    uint32_t specificDataLen;
    std::shared_ptr<CastInfoChunk> info;

    CastMemberChunk(Movie *m) : Chunk(m) {}
    virtual ~CastMemberChunk() = default;
    virtual void read(ReadStream &stream);
};

struct CastInfoChunk : ListChunk {
    uint32_t unk1;
    uint32_t unk2;
    uint32_t flags;
    uint32_t scriptId;

    std::string scriptSrcText;
    std::string name;
    // cProp02;
    // cProp03;
    std::string comment;
    // cProp05;
    // cProp06;
    // cProp07;
    // cProp08;
    // xtraGUID;
    // cProp10;
    // cProp11;
    // cProp12;
    // cProp13;
    // cProp14;
    // cProp15;
    std::string fileFormatID;
    uint32_t created;
    uint32_t modified;
    // cProp19;
    // cProp20;
    // imageCompression;

    CastInfoChunk(Movie *m) : ListChunk(m) {}
    virtual ~CastInfoChunk() = default;
    virtual void read(ReadStream &stream);
};

struct ConfigChunk : Chunk {
    uint16_t len;
    uint16_t fileVersion;
    Rectangle movieRect;
    uint16_t castArrayStart;
    uint16_t castArrayEnd;
    uint16_t directorVersion;

    ConfigChunk(Movie *m) : Chunk(m) {}
    virtual ~ConfigChunk() = default;
    virtual void read(ReadStream &stream);
};

struct InitialMapChunk : Chunk {
    uint32_t memoryMapCount;
    uint32_t memoryMapOffset;

    InitialMapChunk(Movie *m) : Chunk(m) {}
    virtual ~InitialMapChunk() = default;
    virtual void read(ReadStream &stream);
};

struct MemoryMapChunk : Chunk {
    uint16_t unknown0;
    uint16_t unknown1;
    int32_t chunkCountMax;
    int32_t chunkCountUsed;
    int32_t junkPointer;
    int32_t unknown2;
    int32_t freePointer;
    std::vector<MemoryMapEntry> mapArray;

    MemoryMapChunk(Movie *m) : Chunk(m) {}
    virtual ~MemoryMapChunk() = default;
    virtual void read(ReadStream &stream);
};

struct MetaChunk : Chunk {
    uint32_t codec;

    MetaChunk(Movie *m) : Chunk(m) {}
    virtual ~MetaChunk() = default;
    virtual void read(ReadStream &stream);
};

struct ScriptChunk : Chunk {
    uint32_t totalLength;
    uint32_t totalLength2;
    uint16_t headerLength;
    uint16_t scriptNumber;
    uint16_t scriptBehavior;

    uint16_t handlerVectorsCount;
    uint32_t handlerVectorsOffset;
    uint32_t handlerVectorsSize;
    uint16_t propertiesCount;
    uint32_t propertiesOffset;
    uint16_t globalsCount;
    uint32_t globalsOffset;
    uint16_t handlersCount;
    uint32_t handlersOffset;
    uint16_t literalsCount;
    uint32_t literalsOffset;
    uint32_t literalsDataCount;
    uint32_t literalsDataOffset;

    std::vector<int16_t> propertyNameIDs;
    std::vector<int16_t> globalNameIDs;

    std::vector<std::string> propertyNames;
    std::vector<std::string> globalNames;
    std::vector<std::unique_ptr<Handler>> handlers;
    std::vector<LiteralStore> literals;
    std::weak_ptr<ScriptContextChunk> context;

    ScriptChunk(Movie *m) : Chunk(m) {}
    virtual ~ScriptChunk() = default;
    virtual void read(ReadStream &stream);
    std::vector<int16_t> readVarnamesTable(ReadStream &stream, uint16_t count, uint32_t offset);
    void readNames(const std::vector<std::string> &names);
    void translate(const std::vector<std::string> &names);
    std::string toString();
};

struct ScriptContextChunk : Chunk {
    int32_t unknown0;
    int32_t unknown1;
    uint32_t entryCount;
    uint32_t entryCount2;
    uint16_t entriesOffset;
    int16_t unknown2;
    int32_t unknown3;
    int32_t unknown4;
    int32_t unknown5;
    int32_t lnamSectionID;
    uint16_t validCount;
    uint16_t flags;
    int16_t freePointer;

    std::shared_ptr<ScriptNamesChunk> lnam;
    std::vector<ScriptContextMapEntry> sectionMap;
    std::vector<std::shared_ptr<ScriptChunk>> scripts;

    ScriptContextChunk(Movie *m) : Chunk(m) {}
    virtual ~ScriptContextChunk() = default;
    virtual void read(ReadStream &stream);
};

struct ScriptNamesChunk : Chunk {
    int32_t unknown0;
    int32_t unknown1;
    uint32_t len1;
    uint32_t len2;
    uint16_t namesOffset;
    uint16_t namesCount;
    std::vector<std::string> names;

    ScriptNamesChunk(Movie *m) : Chunk(m) {}
    virtual ~ScriptNamesChunk() = default;
    virtual void read(ReadStream &stream);
};

}

#endif
