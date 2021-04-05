#include "castmember.h"
#include "chunk.h"
#include "lingo.h"
#include "movie.h"
#include "stream.h"
#include "subchunk.h"

namespace ProjectorRays {

/* CastChunk */

void CastChunk::read(ReadStream &stream) {
    stream.endianness = kBigEndian;
    while (!stream.eof()) {
        auto id = stream.readInt32();
        memberIDs.push_back(id);
    }
}

void CastChunk::populate(const std::string &castName, int32_t id, uint16_t minMember) {
    name = castName;

    for (const auto &entry : movie->keyTable->entries) {
        if (entry.castID == id && (entry.fourCC == FOURCC('L', 'c', 't', 'x') || entry.fourCC == FOURCC('L', 'c', 't', 'X'))) {
            lctx = std::static_pointer_cast<ScriptContextChunk>(movie->getChunk(entry.fourCC, entry.sectionID));
            break;
        }
    }

    for (size_t i = 0; i < memberIDs.size(); i++) {
        int32_t sectionID = memberIDs[i];
        if (sectionID > 0) {
            auto member = std::static_pointer_cast<CastMemberChunk>(movie->getChunk(FOURCC('C', 'A', 'S', 't'), sectionID));
            member->id = i + minMember;
            if (lctx && (lctx->scripts.find(member->info->scriptId) != lctx->scripts.end())) {
                member->script = lctx->scripts[member->info->scriptId].get();
                member->script->member = member.get();
            }
            members[member->id] = std::move(member);
        }
    }
}

/* CastListChunk */

void CastListChunk::read(ReadStream &stream) {
    stream.endianness = kBigEndian;

    dataOffset = stream.readUint32();
    unk0 = stream.readUint16();
    castCount = stream.readUint16();
    itemsPerCast = stream.readUint16();
    unk1 = stream.readUint16();

    readOffsetTable(stream);

    entries.resize(castCount);
    for (int i = 0; i < castCount; i++) {
        if (itemsPerCast >= 1)
            entries[i].name = readPascalString(stream, i * itemsPerCast + 1);
        if (itemsPerCast >= 2)
            entries[i].filePath = readPascalString(stream, i * itemsPerCast + 2);
        if (itemsPerCast >= 3)
            entries[i].preloadSettings = readUint16(stream, i * itemsPerCast + 3);
        if (itemsPerCast >= 4) {
            auto item = readBytes(stream, i * itemsPerCast + 4);
            entries[i].minMember = item->readUint16();
            entries[i].maxMember = item->readUint16();
            entries[i].id = item->readInt32();
        }
    }
}

/* CastMemberChunk */

void CastMemberChunk::read(ReadStream &stream) {
    stream.endianness = kBigEndian;

    std::unique_ptr<ReadStream> specificStream;

    if (movie->version >= 500) {
        type = static_cast<MemberType>(stream.readUint32());
        infoLen = stream.readUint32();
        specificDataLen = stream.readUint32();

        // info
        std::unique_ptr<ReadStream> infoStream = stream.readBytes(infoLen);
        info = std::make_shared<CastInfoChunk>(movie);
        info->read(*infoStream);

        // specific data
        specificStream = stream.readBytes(specificDataLen);
    } else {
        specificDataLen = stream.readUint16();
        infoLen = stream.readUint32();

        // these bytes are common but stored in the specific data
        uint32_t specificDataLeft = specificDataLen;
        type = static_cast<MemberType>(stream.readUint8());
        specificDataLeft -= 1;
        if (specificDataLeft) {
            /* uint8_t flags1 = */ stream.readUint8();
            specificDataLeft -= 1;
        }

        // specific data
        specificStream = stream.readBytes(specificDataLeft);

        // info
        std::unique_ptr<ReadStream> infoStream = stream.readBytes(infoLen);
        info = std::make_shared<CastInfoChunk>(movie);
        info->read(*infoStream);
    }

    switch (type) {
    case kScriptMember:
        member = std::make_unique<ScriptMember>(movie);
        break;
    default:
        member = std::make_unique<CastMember>(movie, type);
        break;
    }
    member->read(*specificStream);
}

/* CastInfoChunk */

void CastInfoChunk::read(ReadStream &stream) {
    dataOffset = stream.readUint32();
    unk1 = stream.readUint32();
    unk2 = stream.readUint32();
    flags = stream.readUint32();
    scriptId = stream.readUint32();

    readOffsetTable(stream);

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

/* ConfigChunk */

void ConfigChunk::read(ReadStream &stream) {
    stream.endianness = kBigEndian;

    len = stream.readUint16();
    fileVersion = stream.readUint16();
    movieRect.read(stream);

    minMember = stream.readUint16();
    maxMember = stream.readUint16();

    stream.seek(36);
    directorVersion = stream.readUint16();
}

/* InitialMapChunk */

void InitialMapChunk::read(ReadStream &stream) {
    memoryMapCount = stream.readUint32();
    memoryMapOffset = stream.readUint32();
}

/* KeyTableChunk */

void KeyTableChunk::read(ReadStream &stream) {
    unknown0 = stream.readUint16();
    unknown1 = stream.readUint16();
    entryCount = stream.readUint32();
    unknown2 = stream.readUint32();

    entries.resize(entryCount);
    for (auto &entry : entries) {
        entry.read(stream);
    }
}

/* ListChunk */

void ListChunk::read(ReadStream &stream) {
    dataOffset = stream.readUint32();
    readOffsetTable(stream);
}

void ListChunk::readOffsetTable(ReadStream &stream) {
    stream.seek(dataOffset);
    offsetTableLen = stream.readUint16();
    offsetTable.resize(offsetTableLen + 1);
    for (uint16_t i = 0; i < offsetTableLen; i++) {
        offsetTable[i] = stream.readUint32();
    }
    offsetTable[offsetTableLen] = stream.len();
    finalDataLen = stream.readUint32();
    listOffset = stream.pos();
}

std::unique_ptr<ReadStream> ListChunk::readBytes(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return nullptr;

    auto length = offsetTable[index + 1] - offsetTable[index];
    stream.seek(listOffset + offsetTable[index]);
    return stream.readBytes(length);
}

std::string ListChunk::readCString(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return "";

    auto length = offsetTable[index + 1] - offsetTable[index];
    stream.seek(listOffset + offsetTable[index]);
    return stream.readString(length);
}

std::string ListChunk::readPascalString(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return "";

    auto length = offsetTable[index + 1] - offsetTable[index];
    if (length == 0)
        return "";

    stream.seek(listOffset + offsetTable[index]);
    return stream.readPascalString();
}

uint16_t ListChunk::readUint16(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return 0;

    stream.seek(listOffset + offsetTable[index]);
    return stream.readUint16();
}

uint32_t ListChunk::readUint32(ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return 0;

    stream.seek(listOffset + offsetTable[index]);
    return stream.readUint32();
}

/* MemoryMapChunk */

void MemoryMapChunk::read(ReadStream &stream) {
    headerLength = stream.readUint16();
    entryLength = stream.readUint16();
    chunkCountMax = stream.readInt32();
    chunkCountUsed = stream.readInt32();
    junkHead = stream.readInt32();
    junkHead2 = stream.readInt32();
    freeHead = stream.readInt32();
    mapArray.resize(chunkCountUsed);
    for (auto &entry : mapArray) {
        entry.read(stream);
    }
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
        handler = std::make_unique<Handler>(this);
        handler->readRecord(stream);
    }
    for (const auto &handler : handlers) {
        handler->readData(stream);
    }

    stream.seek(literalsOffset);
    literals.resize(literalsCount);
    for (auto &literal : literals) {
        literal.readRecord(stream, movie->version);
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

std::string ScriptChunk::getName(int id) {
    return context->getName(id);
}

void ScriptChunk::setContext(ScriptContextChunk *ctx) {
    this->context = ctx;
    for (auto nameID : propertyNameIDs) {
        propertyNames.push_back(getName(nameID));
    }
    for (auto nameID : globalNameIDs) {
        globalNames.push_back(getName(nameID));
    }
    for (const auto &handler : handlers) {
        handler->readNames();
    }
}

void ScriptChunk::translate() {
    for (const auto &handler : handlers) {
        handler->translate();
    }
}

std::string ScriptChunk::varDeclarations() {
    std::string res = "";
    if (propertyNames.size() > 0) {
        res += "property ";
        for (size_t i = 0; i < propertyNames.size(); i++) {
            if (i > 0)
                res += ", ";
            res += propertyNames[i];
        }
        res += "\n";
    }
    if (globalNames.size() > 0) {
        res += "global ";
        for (size_t i = 0; i < globalNames.size(); i++) {
            if (i > 0)
                res += ", ";
            res += globalNames[i];
        }
        res += "\n";
    }
    return res;
}

std::string ScriptChunk::scriptText() {
    std::string res = varDeclarations();
    for (size_t i = 0; i < handlers.size(); i++) {
        if (res.size() > 0)
            res += "\n";
        res += handlers[i]->ast->toString(false);
    }
    return res;
}

std::string ScriptChunk::bytecodeText() {
    std::string res = varDeclarations();
    for (size_t i = 0; i < handlers.size(); i++) {
        if (res.size() > 0)
            res += "\n";
        res += handlers[i]->bytecodeText();
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

    lnam = std::static_pointer_cast<ScriptNamesChunk>(movie->getChunk(FOURCC('L', 'n', 'a', 'm'), lnamSectionID));
    for (uint32_t i = 1; i <= entryCount; i++) {
        auto section = sectionMap[i - 1];
        if (section.sectionID > -1) {
            auto script = std::static_pointer_cast<ScriptChunk>(movie->getChunk(FOURCC('L', 's', 'c', 'r'), section.sectionID));
            script->setContext(this);
            scripts[i] = script;
        }
    }

    for (auto it = scripts.begin(); it != scripts.end(); ++it) {
        it->second->translate();
    }
}

std::string ScriptContextChunk::getName(int id) {
    return lnam->getName(id);
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

std::string ScriptNamesChunk::getName(int id) {
    if (-1 < id && (unsigned)id < names.size())
        return names[id];
    return "UNKNOWN_NAME_" + std::to_string(id);
}

}
