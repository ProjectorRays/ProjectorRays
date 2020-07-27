#include "chunk.h"
#include "lingo.h"
#include "stream.h"
#include "subchunk.h"

namespace ProjectorRays {

/* CastAssociationsChunk */

void CastAssociationsChunk::read(ReadStream &stream) {
    stream.endianness = kBigEndian;
    while (!stream.eof()) {
        auto id = stream.readUint32();
        if (id > 0) {
            entries.push_back(id);
        }
    }
}

/* CastKeyChunk */

void CastKeyChunk::read(ReadStream &stream) {
    unknown0 = stream.readUint16();
    unknown1 = stream.readUint16();
    entryCount = stream.readUint32();
    unknown2 = stream.readUint32();

    entries.resize(entryCount);
    for (auto &entry : entries) {
        entry.read(stream);
        // if (entry.sectionID > 0) {
        //     entries.push(entry);
        // }
    }
}

/* CastListChunk */

void CastListChunk::read(ReadStream &stream) {
    stream.endianness = kBigEndian;

    unknown0 = stream.readUint32();
    castCount = stream.readUint32();
    unknown1 = stream.readUint16();
    arraySize = stream.readUint32();
    // offsetTable = stream.readBytes(arraySize * 4);
    stream.skip(arraySize * 4);
    castEntriesLength = stream.readUint32();
    entries.resize(castCount);
    for (auto &entry : entries) {
        entry.read(stream);
    }
}

/* CastMemberChunk */

void CastMemberChunk::read(ReadStream &stream) {
    stream.endianness = kBigEndian;

    type = stream.readUint32();
    infoLen = stream.readUint32();
    specificDataLen = stream.readUint32();

    // info
    std::unique_ptr<ReadStream> infoStream = stream.readBytes(infoLen);
    info = std::make_shared<CastInfoChunk>();
    info->read(*infoStream);

    // specific data
    // TODO
}

/* CastInfoChunk */

void CastInfoChunk::read(ReadStream &stream) {
    dataOffset = stream.readUint32();
    unk1 = stream.readUint32();
    unk2 = stream.readUint32();
    flags = stream.readUint32();
    scriptId = stream.readUint32();

    stream.seek(dataOffset);
    offsetTableLen = stream.readUint16();
    offsetTable.resize(offsetTableLen);
    for (auto &entry : offsetTable) {
        entry = stream.readUint32();
    }
    finalDataLen = stream.readUint32();
    listOffset = stream.pos();

    scriptSrcText = readCString(stream, 0);
    name = readPascalString(stream, 1);
    // cProp02 = readProperty(stream, 2);
    // cProp03 = readProperty(stream, 3);
    comment = readCString(stream, 4);
    // cProp05 = readProperty(stream, 5);
    // cProp06 = readProperty(stream, 6);
    // cProp07 = readProperty(stream, 7);
    // cProp08 = readProperty(stream, 8);
    // xtraGUID = readProperty(stream, 9);
    // cProp10 = readProperty(stream, 10);
    // cProp11 = readProperty(stream, 11);
    // cProp12 = readProperty(stream, 12);
    // cProp13 = readProperty(stream, 13);
    // cProp14 = readProperty(stream, 14);
    // cProp15 = readProperty(stream, 15);
    fileFormatID = readCString(stream, 16);
    created = readUint32(stream, 17);
    modified = readUint32(stream, 18);
    // cProp19 = readProperty(stream, 19);
    // cProp20 = readProperty(stream, 20);
    // imageCompression = readProperty(stream, 21);
}

std::string CastInfoChunk::readCString(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen - 1)
        return "";

    auto length = offsetTable[index + 1] - offsetTable[index];
    stream.seek(listOffset + offsetTable[index]);
    return stream.readString(length);
}

std::string CastInfoChunk::readPascalString(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen - 1)
        return "";

    stream.seek(listOffset + offsetTable[index]);
    return stream.readPascalString();
}

uint32_t CastInfoChunk::readUint32(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen - 1)
        return 0;

    stream.seek(listOffset + offsetTable[index]);
    return stream.readUint32();
}

/* InitialMapChunk */

void InitialMapChunk::read(ReadStream &stream) {
    memoryMapCount = stream.readUint32();
    memoryMapOffset = stream.readUint32();
}

/* MemoryMapChunk */

void MemoryMapChunk::read(ReadStream &stream) {
    unknown0 = stream.readUint16();
    unknown1 = stream.readUint16();
    // possible one of the unknown mmap entries determines why an unused item is there?
    // it also seems code comments can be inserted after mmap after chunkCount is over, it may warrant investigation
    chunkCountMax = stream.readInt32();
    chunkCountUsed = stream.readInt32();
    junkPointer = stream.readInt32();
    unknown2 = stream.readInt32();
    freePointer = stream.readInt32();
    mapArray.resize(chunkCountUsed);
    // seems chunkCountUsed is used here, so what is chunkCount for?
    // EDIT: chunkCountMax is maximum allowed chunks before new mmap created!
    for (auto &entry : mapArray) {
        // don't actually generate new chunk objects here, just read in data
        entry.read(stream);
        // we don't care about free or junk chunks
        // if (entry.name !== "free" && entry.name !== "junk") {
        //     mapArray.push(entry);
        // }
    }
}

/* MetaChunk */

void MetaChunk::read(ReadStream &stream) {
    codec = stream.readUint32();
}

/* ScriptChunk */

void ScriptChunk::read(ReadStream &stream) {
    stream.seek(8);
    // Lingo scripts are always big endian regardless of file endianness
    stream.endianness = kBigEndian;
    totalLength = stream.readUint32();
    totalLength2 = stream.readUint32();
    headerLength = stream.readUint16();
    scriptNumber = stream.readUint16();
    stream.seek(38);
    scriptBehavior = stream.readUint32();
    stream.seek(50);
    handlerVectorsCount = stream.readUint16();
    handlerVectorsOffset = stream.readUint32();
    handlerVectorsSize = stream.readUint32();
    propertiesCount = stream.readUint16();
    propertiesOffset = stream.readUint32();
    globalsCount = stream.readUint16();
    globalsOffset = stream.readUint32();
    handlersCount = stream.readUint16();
    handlersOffset = stream.readUint32();
    literalsCount = stream.readUint16();
    literalsOffset = stream.readUint32();
    literalsDataCount = stream.readUint32();
    literalsDataOffset = stream.readUint32();
    propertyNameIDs = readVarnamesTable(stream, propertiesCount, propertiesOffset);
    globalNameIDs = readVarnamesTable(stream, globalsCount, globalsOffset);

    stream.seek(handlersOffset);
    handlers.resize(handlersCount);
    for (auto &handler : handlers) {
        handler = std::make_unique<Handler>(weak_from_this());
        handler->readRecord(stream);
    }
    for (const auto &handler : handlers) {
        handler->readData(stream);
    }

    stream.seek(literalsOffset);
    literals.resize(literalsCount);
    for (auto &literal : literals) {
        literal.readRecord(stream);
    }
    for (auto &literal : literals) {
        literal.readData(stream, literalsDataOffset);
    }
}

std::vector<int16_t> ScriptChunk::readVarnamesTable(ReadStream &stream, uint16_t count, uint32_t offset) {
    stream.seek(offset);
    std::vector<int16_t> nameIDs(count);
    for (uint16_t i = 0; i < count; i++) {
        nameIDs[i] = stream.readInt16();
    }
    return nameIDs;
}

void ScriptChunk::readNames(const std::vector<std::string> &names) {
    for (auto nameID : propertyNameIDs) {
        if (0 <= nameID && nameID < names.size())
            propertyNames.push_back(names[nameID]);
        else
            propertyNames.push_back("UNKNOWN");
    }
    for (auto nameID : globalNameIDs) {
        if (0 <= nameID && nameID < names.size())
            globalNames.push_back(names[nameID]);
        else
            globalNames.push_back("UNKNOWN");
    }
    for (const auto &handler : handlers) {
        handler->readNames(names);
    }
}

void ScriptChunk::translate(const std::vector<std::string> &names) {
    for (const auto &handler : handlers) {
        handler->translate(names);
    }
}

std::string ScriptChunk::toString() {
    std::string res = "";
    for (size_t i = 0; i < handlers.size(); i++) {
        if (i > 0)
            res += "\n\n";
        res += handlers[i]->ast->toString();
    }
    return res;
}

/* ScriptContextChunk */

void ScriptContextChunk::read(ReadStream &stream) {
    // Lingo scripts are always big endian regardless of file endianness
    stream.endianness = kBigEndian;

    unknown0 = stream.readInt32();
    unknown1 = stream.readInt32();
    entryCount = stream.readUint32();
    entryCount2 = stream.readUint32();
    entriesOffset = stream.readUint16();
    unknown2 = stream.readInt16();
    unknown3 = stream.readInt32();
    unknown4 = stream.readInt32();
    unknown5 = stream.readInt32();
    lnamSectionID = stream.readInt32();
    validCount = stream.readUint16();
    flags = stream.readUint16();
    freePointer = stream.readInt16();

    stream.seek(entriesOffset);
    sectionMap.resize(entryCount);
    for (auto &entry : sectionMap) {
        entry.read(stream);
    }
}

/* ScriptNamesChunk */

void ScriptNamesChunk::read(ReadStream &stream) {
    // Lingo scripts are always big endian regardless of file endianness
    stream.endianness = kBigEndian;

    unknown0 = stream.readInt32();
    unknown1 = stream.readInt32();
    len1 = stream.readUint32();
    len2 = stream.readUint32();
    namesOffset = stream.readUint16();
    namesCount = stream.readUint16();

    stream.seek(namesOffset);
    names.resize(namesCount);
    for (auto &name : names) {
        auto length = stream.readUint8();
        name = stream.readString(length);
    }
}

}
