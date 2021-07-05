#include "common/stream.h"
#include "director/castmember.h"
#include "director/chunk.h"
#include "director/lingo.h"
#include "director/dirfile.h"
#include "director/subchunk.h"
#include "director/util.h"

namespace Director {

/* Chunk */

void to_json(ordered_json &j, const Chunk &c) {
    switch (c.chunkType) {
    case kCastChunk:
        to_json(j, static_cast<const CastChunk &>(c));
        break;
    case kCastListChunk:
        to_json(j, static_cast<const CastListChunk &>(c));
        break;
    case kCastMemberChunk:
        to_json(j, static_cast<const CastMemberChunk &>(c));
        break;
    case kCastInfoChunk:
        to_json(j, static_cast<const CastInfoChunk &>(c));
        break;
    case kConfigChunk:
        to_json(j, static_cast<const ConfigChunk &>(c));
        break;
    case kInitialMapChunk:
        to_json(j, static_cast<const InitialMapChunk &>(c));
        break;
    case kKeyTableChunk:
        to_json(j, static_cast<const KeyTableChunk &>(c));
        break;
    case kMemoryMapChunk:
        to_json(j, static_cast<const MemoryMapChunk &>(c));
        break;
    case kScriptChunk:
        to_json(j, static_cast<const ScriptChunk &>(c));
        break;
    case kScriptContextChunk:
        to_json(j, static_cast<const ScriptContextChunk &>(c));
        break;
    case kScriptNamesChunk:
        to_json(j, static_cast<const ScriptNamesChunk &>(c));
        break;
    }
}

/* CastChunk */

void CastChunk::read(Common::ReadStream &stream) {
    stream.endianness = Common::kBigEndian;
    while (!stream.eof()) {
        auto id = stream.readInt32();
        memberIDs.push_back(id);
    }
}

void to_json(ordered_json &j, const CastChunk &c) {
    j["memberIDs"] = c.memberIDs;
}

void CastChunk::populate(const std::string &castName, int32_t id, uint16_t minMember) {
    name = castName;

    for (const auto &entry : dir->keyTable->entries) {
        if (entry.castID == id
                && (entry.fourCC == FOURCC('L', 'c', 't', 'x') || entry.fourCC == FOURCC('L', 'c', 't', 'X'))
                && dir->chunkExists(entry.fourCC, entry.sectionID)) {
            lctx = std::static_pointer_cast<ScriptContextChunk>(dir->getChunk(entry.fourCC, entry.sectionID));
            break;
        }
    }

    for (size_t i = 0; i < memberIDs.size(); i++) {
        int32_t sectionID = memberIDs[i];
        if (sectionID > 0) {
            auto member = std::static_pointer_cast<CastMemberChunk>(dir->getChunk(FOURCC('C', 'A', 'S', 't'), sectionID));
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

void CastListChunk::read(Common::ReadStream &stream) {
    stream.endianness = Common::kBigEndian;

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

void to_json(ordered_json &j, const CastListChunk &c) {
    j["dataOffset"] = c.dataOffset;
    j["unk0"] = c.unk0;
    j["castCount"] = c.itemsPerCast;
    j["itemsPerCast"] = c.itemsPerCast;
    j["unk1"] = c.unk1;
    j["entries"] = c.entries;
}

/* CastMemberChunk */

void CastMemberChunk::read(Common::ReadStream &stream) {
    stream.endianness = Common::kBigEndian;

    std::unique_ptr<Common::ReadStream> specificStream;

    if (dir->version >= 500) {
        type = static_cast<MemberType>(stream.readUint32());
        infoLen = stream.readUint32();
        specificDataLen = stream.readUint32();

        // info
        std::unique_ptr<Common::ReadStream> infoStream = stream.readBytes(infoLen);
        info = std::make_shared<CastInfoChunk>(dir);
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
        std::unique_ptr<Common::ReadStream> infoStream = stream.readBytes(infoLen);
        info = std::make_shared<CastInfoChunk>(dir);
        info->read(*infoStream);
    }

    switch (type) {
    case kScriptMember:
        member = std::make_unique<ScriptMember>(dir);
        break;
    default:
        member = std::make_unique<CastMember>(dir, type);
        break;
    }
    member->read(*specificStream);
}

void to_json(ordered_json &j, const CastMemberChunk &c) {
    j["type"] = c.type;
    j["infoLen"] = c.infoLen;
    j["specificDataLen"] = c.specificDataLen;
    j["info"] = *c.info;
    j["member"] = *c.member;
}

/* CastInfoChunk */

void CastInfoChunk::read(Common::ReadStream &stream) {
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

void to_json(ordered_json &j, const CastInfoChunk &c) {
    j["dataOffset"] = c.dataOffset;
    j["unk1"] = c.unk1;
    j["unk2"] = c.unk2;
    j["flags"] = c.flags;
    j["scriptId"] = c.scriptId;
    j["scriptSrcText"] = c.scriptSrcText;
    j["name"] = c.name;
    j["comment"] = c.comment;
    j["fileFormatID"] = c.fileFormatID;
    j["created"] = c.created;
    j["modified"] = c.modified;
}

/* ConfigChunk */

void ConfigChunk::read(Common::ReadStream &stream) {
    stream.endianness = Common::kBigEndian;

    len = stream.readUint16();
    fileVersion = stream.readUint16();
    movieRect.read(stream);

    minMember = stream.readUint16();
    maxMember = stream.readUint16();

    stream.seek(36);
    directorVersion = stream.readUint16();
}

void to_json(ordered_json &j, const ConfigChunk &c) {
    j["len"] = c.len;
    j["fileVersion"] = c.fileVersion;
    j["movieRect"] = c.movieRect;
    j["minMember"] = c.minMember;
    j["maxMember"] = c.maxMember;
    j["directorVersion"] = c.directorVersion;
}

/* InitialMapChunk */

void InitialMapChunk::read(Common::ReadStream &stream) {
    one = stream.readUint32();
    mmapOffset = stream.readUint32();
    version = stream.readUint32();
    unused1 = stream.readUint32();
    unused2 = stream.readUint32();
    unused3 = stream.readUint32();
}

void to_json(ordered_json &j, const InitialMapChunk &c) {
    j["one"] = c.one;
    j["mmapOffset"] = c.mmapOffset;
    j["version"] = c.version;
    j["unused1"] = c.unused1;
    j["unused2"] = c.unused2;
    j["unused3"] = c.unused3;
}

/* KeyTableChunk */

void KeyTableChunk::read(Common::ReadStream &stream) {
    entrySize = stream.readUint16();
    entrySize2 = stream.readUint16();
    entryCount = stream.readUint32();
    usedCount = stream.readUint32();

    entries.resize(entryCount);
    for (auto &entry : entries) {
        entry.read(stream);
    }
}

void to_json(ordered_json &j, const KeyTableChunk &c) {
    j["entrySize"] = c.entrySize;
    j["entrySize2"] = c.entrySize2;
    j["entryCount"] = c.entryCount;
    j["usedCount"] = c.usedCount;
    j["entries"] = c.entries;
}

/* ListChunk */

void ListChunk::read(Common::ReadStream &stream) {
    dataOffset = stream.readUint32();
    readOffsetTable(stream);
}

void ListChunk::readOffsetTable(Common::ReadStream &stream) {
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

std::unique_ptr<Common::ReadStream> ListChunk::readBytes(Common::ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return nullptr;

    auto length = offsetTable[index + 1] - offsetTable[index];
    stream.seek(listOffset + offsetTable[index]);
    return stream.readBytes(length);
}

std::string ListChunk::readCString(Common::ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return "";

    auto length = offsetTable[index + 1] - offsetTable[index];
    stream.seek(listOffset + offsetTable[index]);
    return stream.readString(length);
}

std::string ListChunk::readPascalString(Common::ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return "";

    auto length = offsetTable[index + 1] - offsetTable[index];
    if (length == 0)
        return "";

    stream.seek(listOffset + offsetTable[index]);
    return stream.readPascalString();
}

uint16_t ListChunk::readUint16(Common::ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return 0;

    stream.seek(listOffset + offsetTable[index]);
    return stream.readUint16();
}

uint32_t ListChunk::readUint32(Common::ReadStream &stream, uint16_t index) {
    if (index >= offsetTableLen)
        return 0;

    stream.seek(listOffset + offsetTable[index]);
    return stream.readUint32();
}

/* MemoryMapChunk */

void MemoryMapChunk::read(Common::ReadStream &stream) {
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

void to_json(ordered_json &j, const MemoryMapChunk &c) {
    j["headerLength"] = c.headerLength;
    j["entryLength"] = c.entryLength;
    j["chunkCountMax"] = c.chunkCountMax;
    j["chunkCountUsed"] = c.chunkCountUsed;
    j["junkHead"] = c.junkHead;
    j["junkHead2"] = c.junkHead2;
    j["freeHead"] = c.freeHead;
    j["mapArray"] = c.mapArray;
}

/* ScriptChunk */

void ScriptChunk::read(Common::ReadStream &stream) {
    stream.seek(8);
    // Lingo scripts are always big endian regardless of file endianness
    stream.endianness = Common::kBigEndian;
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
        literal.readRecord(stream, dir->version);
    }
    for (auto &literal : literals) {
        literal.readData(stream, literalsDataOffset);
    }
}

std::vector<int16_t> ScriptChunk::readVarnamesTable(Common::ReadStream &stream, uint16_t count, uint32_t offset) {
    stream.seek(offset);
    std::vector<int16_t> nameIDs(count);
    for (uint16_t i = 0; i < count; i++) {
        nameIDs[i] = stream.readInt16();
    }
    return nameIDs;
}

void to_json(ordered_json &j, const ScriptChunk &c) {
    j["totalLength"] = c.totalLength;
    j["totalLength2"] = c.totalLength2;
    j["headerLength"] = c.headerLength;
    j["scriptNumber"] = c.scriptNumber;
    j["scriptBehavior"] = c.scriptBehavior;
    j["handlerVectorsCount"] = c.handlerVectorsCount;
    j["handlerVectorsOffset"] = c.handlerVectorsOffset;
    j["handlerVectorsSize"] = c.handlerVectorsSize;
    j["propertiesCount"] = c.propertiesCount;
    j["propertiesOffset"] = c.propertiesOffset;
    j["globalsCount"] = c.globalsCount;
    j["globalsOffset"] = c.globalsOffset;
    j["handlersCount"] = c.handlersCount;
    j["handlersOffset"] = c.handlersOffset;
    j["literalsCount"] = c.literalsCount;
    j["literalsOffset"] = c.literalsOffset;
    j["literalsDataCount"] = c.literalsDataCount;
    j["literalsDataOffset"] = c.literalsDataOffset;
    j["propertyNameIDs"] = c.propertyNameIDs;
    j["globalNameIDs"] = c.globalNameIDs;
    ordered_json handlers = ordered_json::array();
    for (const auto &handler : c.handlers)
        handlers.push_back(*handler);
    j["handlers"] = handlers;
    j["literals"] = c.literals;
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
        res += handlers[i]->ast->toString(dir->dotSyntax, false);
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

void ScriptContextChunk::read(Common::ReadStream &stream) {
    // Lingo scripts are always big endian regardless of file endianness
    stream.endianness = Common::kBigEndian;

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

    lnam = std::static_pointer_cast<ScriptNamesChunk>(dir->getChunk(FOURCC('L', 'n', 'a', 'm'), lnamSectionID));
    for (uint32_t i = 1; i <= entryCount; i++) {
        auto section = sectionMap[i - 1];
        if (section.sectionID > -1) {
            auto script = std::static_pointer_cast<ScriptChunk>(dir->getChunk(FOURCC('L', 's', 'c', 'r'), section.sectionID));
            script->setContext(this);
            scripts[i] = script;
        }
    }

    for (auto it = scripts.begin(); it != scripts.end(); ++it) {
        it->second->translate();
    }
}

void to_json(ordered_json &j, const ScriptContextChunk &c) {
    j["unknown0"] = c.unknown0;
    j["unknown1"] = c.unknown1;
    j["entryCount"] = c.entryCount;
    j["entryCount2"] = c.entryCount2;
    j["entriesOffset"] = c.entriesOffset;
    j["unknown2"] = c.unknown2;
    j["unknown3"] = c.unknown3;
    j["unknown4"] = c.unknown4;
    j["unknown5"] = c.unknown5;
    j["lnamSectionID"] = c.lnamSectionID;
    j["validCount"] = c.validCount;
    j["flags"] = c.flags;
    j["freePointer"] = c.freePointer;
    j["sectionMap"] = c.sectionMap;
}

std::string ScriptContextChunk::getName(int id) {
    return lnam->getName(id);
}

/* ScriptNamesChunk */

void ScriptNamesChunk::read(Common::ReadStream &stream) {
    // Lingo scripts are always big endian regardless of file endianness
    stream.endianness = Common::kBigEndian;

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

void to_json(ordered_json &j, const ScriptNamesChunk &c) {
    j["unknown0"] = c.unknown0;
    j["unknown1"] = c.unknown1;
    j["len1"] = c.len1;
    j["len2"] = c.len2;
    j["namesOffset"] = c.namesOffset;
    j["namesCount"] = c.namesCount;
    j["names"] = c.names;
}

std::string ScriptNamesChunk::getName(int id) {
    if (-1 < id && (unsigned)id < names.size())
        return names[id];
    return "UNKNOWN_NAME_" + std::to_string(id);
}

}
