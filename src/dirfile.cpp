#include <iostream>
#include <sstream>
#include <stdexcept>
#include <boost/format.hpp>

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "chunk.h"
#include "lingo.h"
#include "dirfile.h"
#include "fileio.h"
#include "stream.h"
#include "subchunk.h"
#include "util.h"

namespace Director {

/* DirectorFile */

void DirectorFile::read(Common::ReadStream *s) {
    stream = s;
    stream->endianness = Common::kBigEndian; // we set this properly when we create the RIFX chunk

    // Meta
    auto metaFourCC = stream->readUint32();
    if (metaFourCC == FOURCC('X', 'F', 'I', 'R')) {
        stream->endianness = Common::kLittleEndian;
    }
    stream->readInt32(); // meta length
    codec = stream->readUint32();

    // Codec-dependent map
    if (codec == FOURCC('M', 'V', '9', '3')) {
        readMemoryMap();
    } else if (codec == FOURCC('F', 'G', 'D', 'M') || codec == FOURCC('F', 'G', 'D', 'C')) {
        afterburned = true;
        if (!readAfterburnerMap())
            return;
    } else {
        throw std::runtime_error("Codec unsupported: " + fourCCToString(codec));
    }

    if (!readKeyTable())
        return;
    if (!readConfig())
        return;
    if (!readCasts())
        return;
}

void DirectorFile::readMemoryMap() {
    // Initial map
    std::shared_ptr<InitialMapChunk> imap = std::static_pointer_cast<InitialMapChunk>(readChunk(FOURCC('i', 'm', 'a', 'p')));
    deserializedChunks[1] = imap;

    // Memory map
    stream->seek(imap->mmapOffset);
    std::shared_ptr<MemoryMapChunk> mmap = std::static_pointer_cast<MemoryMapChunk>(readChunk(FOURCC('m', 'm', 'a', 'p')));
    deserializedChunks[2] = mmap;

    for (uint32_t i = 0; i < mmap->mapArray.size(); i++) {
        auto mapEntry = mmap->mapArray[i];

        if (mapEntry.fourCC == FOURCC('f', 'r', 'e', 'e') || mapEntry.fourCC == FOURCC('j', 'u', 'n', 'k'))
            continue;

        std::cout << boost::format("Found RIFX resource index %d: '%s', %d bytes @ pos 0x%08x (%d)\n")
                        % i % fourCCToString(mapEntry.fourCC) % mapEntry.len % mapEntry.offset % mapEntry.offset;

        ChunkInfo info;
        info.id = i;
        info.fourCC = mapEntry.fourCC;
        info.len = mapEntry.len;
        info.uncompressedLen = mapEntry.len;
        info.offset = mapEntry.offset;
        info.compressionType = 0;
        chunkInfo[i] = info;

        chunkIDsByFourCC[mapEntry.fourCC].push_back(i);
    }
}

bool DirectorFile::readAfterburnerMap() {
    uint32_t start, end;

    // File version
    if (stream->readUint32() != FOURCC('F', 'v', 'e', 'r')) {
        std::cout << "readAfterburnerMap(): Fver expected but not found\n";
        return false;
    }

    uint32_t fverLength = stream->readVarInt();
    start = stream->pos();
    uint32_t version = stream->readVarInt();
    std::cout << boost::format("Fver: version: %x\n") % version;
    end = stream->pos();

    if (end - start != fverLength) {
        std::cout << boost::format("readAfterburnerMap(): Expected Fver of length %d but read %d bytes\n")
                        % fverLength % (end - start);
        stream->seek(start + fverLength);
    }

    // Compression types
    if (stream->readUint32() != FOURCC('F', 'c', 'd', 'r')) {
        std::cout << "readAfterburnerMap(): Fcdr expected but not found\n";
        return false;
    }

    uint32_t fcdrLength = stream->readVarInt();
    stream->skip(fcdrLength);

    // Afterburner map
    if (stream->readUint32() != FOURCC('A', 'B', 'M', 'P')) {
        std::cout << "RIFXArchive::readAfterburnerMap(): ABMP expected but not found\n";
        return false;
    }
    uint32_t abmpLength = stream->readVarInt();
    uint32_t abmpEnd = stream->pos() + abmpLength;
    uint32_t abmpCompressionType = stream->readVarInt();
    unsigned long abmpUncompLength = stream->readVarInt();
    unsigned long abmpActualUncompLength = abmpUncompLength;
    std::cout << boost::format("ABMP: length: %d compressionType: %d uncompressedLength: %lu\n")
                    % abmpLength % abmpCompressionType % abmpUncompLength;

    auto abmpStream = stream->readZlibBytes(abmpEnd - stream->pos(), &abmpActualUncompLength);
    if (!abmpStream) {
        std::cout << "RIFXArchive::readAfterburnerMap(): Could not uncompress ABMP\n";
        return false;
    }
    if (abmpUncompLength != abmpActualUncompLength) {
        std::cout << boost::format("ABMP: Expected uncompressed length %lu but got length %lu\n")
                        % abmpUncompLength % abmpActualUncompLength;
    }

    uint32_t abmpUnk1 = abmpStream->readVarInt();
    uint32_t abmpUnk2 = abmpStream->readVarInt();
    uint32_t resCount = abmpStream->readVarInt();
    std::cout << boost::format("ABMP: unk1: %d unk2: %d resCount: %d\n")
                    % abmpUnk1 % abmpUnk2 % resCount;

    for (uint32_t i = 0; i < resCount; i++) {
        uint32_t resId = abmpStream->readVarInt();
        int32_t offset = abmpStream->readVarInt();
        uint32_t compSize = abmpStream->readVarInt();
        uint32_t uncompSize = abmpStream->readVarInt();
        uint32_t compressionType = abmpStream->readVarInt();
        uint32_t tag = abmpStream->readUint32();

        std::cout << boost::format("Found RIFX resource index %d: '%s', %d bytes (%d uncompressed) @ pos 0x%08x (%d), compressionType: %d\n")
                        % resId % fourCCToString(tag) % compSize % uncompSize % offset % offset % compressionType;

        ChunkInfo info;
        info.id = resId;
        info.fourCC = tag;
        info.len = compSize;
        info.uncompressedLen = uncompSize;
        info.offset = offset;
        info.compressionType = compressionType;
        chunkInfo[resId] = info;

        chunkIDsByFourCC[tag].push_back(resId);
    }

    // Initial load segment
    if (chunkInfo.find(2) == chunkInfo.end()) {
        std::cout << "readAfterburnerMap(): Map has no entry for ILS\n";
        return false;
    }
    if (stream->readUint32() != FOURCC('F', 'G', 'E', 'I')) {
        std::cout << "readAfterburnerMap(): FGEI expected but not found\n";
        return false;
    }

    ChunkInfo &ilsInfo = chunkInfo[2];
    uint32_t ilsUnk1 = stream->readVarInt();
    std::cout << boost::format("ILS: length: %d unk1: %d\n") % ilsInfo.len % ilsUnk1;
    _ilsBodyOffset = stream->pos();
    unsigned long ilsActualUncompLength = ilsInfo.uncompressedLen;
    auto ilsStream = stream->readZlibBytes(ilsInfo.len, &ilsActualUncompLength);
    if (!ilsStream) {
        std::cout << "readAfterburnerMap(): Could not uncompress FGEI\n";
        return false;
    }
    if (ilsInfo.uncompressedLen != ilsActualUncompLength) {
        std::cout << boost::format("ILS: Expected uncompressed length %d but got length %lu\n")
                        % ilsInfo.uncompressedLen % ilsActualUncompLength;
    }

    while (!ilsStream->eof()) {
        uint32_t resId = ilsStream->readVarInt();
        ChunkInfo &info = chunkInfo[resId];

        std::cout << boost::format("Loading ILS resource %d: '%s', %d bytes\n")
                        % resId % fourCCToString(info.fourCC) % info.len;

        auto data = ilsStream->copyBytes(info.len);
        if (data) {
            _cachedChunkData[resId] = std::move(data);
        } else {
            std::cout << boost::format("Could not load ILS resource %d\n") % resId;
        }
    }

    return true;
}

bool DirectorFile::readKeyTable() {
    auto info = getFirstChunkInfo(FOURCC('K', 'E', 'Y', '*'));
    if (info) {
        keyTable = std::static_pointer_cast<KeyTableChunk>(getChunk(info->fourCC, info->id));

        for (size_t i = 0; i < keyTable->usedCount; i++) {
            const KeyTableEntry &entry = keyTable->entries[i];
            uint32_t ownerTag = FOURCC('?', '?', '?', '?');
            if (chunkInfo.find(entry.castID) != chunkInfo.end()) {
                ownerTag = chunkInfo[entry.castID].fourCC;
            }
            std::cout << boost::format("KEY* entry %d: '%s' @ %d owned by '%s' @ %d\n")
                % i % fourCCToString(entry.fourCC) % entry.sectionID % fourCCToString(ownerTag) % entry.castID;
        }

        return true;
    }

    std::cout << "No key chunk!\n";
    return false;
}

bool DirectorFile::readConfig() {
    auto info = getFirstChunkInfo(FOURCC('V', 'W', 'C', 'F'));
    if (!info)
        info = getFirstChunkInfo(FOURCC('D', 'R', 'C', 'F'));

    if (info) {
        config = std::static_pointer_cast<ConfigChunk>(getChunk(info->fourCC, info->id));
        version = humanVersion(config->directorVersion);
        std::cout << "Director version: " + std::to_string(version) + "\n";
        dotSyntax = (version >= 700); // TODO: Check for verbose/dot syntax opcodes, allow users to toggle this

        return true;
    }

    std::cout << "No config chunk!\n";
    return false;
}

bool DirectorFile::readCasts() {
    bool internal = true;

    if (version >= 500) {
        auto info = getFirstChunkInfo(FOURCC('M', 'C', 's', 'L'));
        if (info) {
            auto castList = std::static_pointer_cast<CastListChunk>(getChunk(info->fourCC, info->id));
            for (const auto &castEntry : castList->entries) {
                std::cout << "Cast: " + castEntry.name + "\n";
                int32_t sectionID = -1;
                for (const auto &keyEntry : keyTable->entries) {
                    if (keyEntry.castID == castEntry.id && keyEntry.fourCC == FOURCC('C', 'A', 'S', '*')) {
                        sectionID = keyEntry.sectionID;
                        break;
                    }
                }
                if (sectionID > 0) {
                    auto cast = std::static_pointer_cast<CastChunk>(getChunk(FOURCC('C', 'A', 'S', '*'), sectionID));
                    cast->populate(castEntry.name, castEntry.id, castEntry.minMember);
                    casts.push_back(std::move(cast));
                }
            }

            return true;
        } else {
            internal = false;
        }
    }

    auto info = getFirstChunkInfo(FOURCC('C', 'A', 'S', '*'));
    if (info) {
        auto cast = std::static_pointer_cast<CastChunk>(getChunk(info->fourCC, info->id));
        cast->populate(internal ? "Internal" : "External", 1024, config->minMember);
        casts.push_back(std::move(cast));

        return true;
    }

    std::cout << "No cast!\n";
    return false;

    return false;
}

const ChunkInfo *DirectorFile::getFirstChunkInfo(uint32_t fourCC) {
    auto &chunkIDs = chunkIDsByFourCC[fourCC];
    if (chunkIDs.size() > 0) {
        return &chunkInfo[chunkIDs[0]];
    }
    return nullptr;
}

bool DirectorFile::chunkExists(uint32_t fourCC, int32_t id) {
    if (chunkInfo.find(id) == chunkInfo.end())
        return false;
    
    if (fourCC != chunkInfo[id].fourCC)
        return false;

    return true;
}

std::shared_ptr<Chunk> DirectorFile::getChunk(uint32_t fourCC, int32_t id) {
    if (deserializedChunks.find(id) != deserializedChunks.end())
        return deserializedChunks[id];
    
    std::unique_ptr<Common::ReadStream> chunkData = getChunkData(fourCC, id);
    if (!chunkData) {
        throw std::runtime_error(boost::str(
            boost::format("No data for chunk %d") % id
        ));
    }
    std::shared_ptr<Chunk> chunk = makeChunk(fourCC, *chunkData);

    deserializedChunks[id] = chunk;

    return chunk;
}

std::unique_ptr<Common::ReadStream> DirectorFile::getChunkData(uint32_t fourCC, int32_t id) {
    if (chunkInfo.find(id) == chunkInfo.end())
        throw std::runtime_error("Could not find chunk " + std::to_string(id));

    auto &info = chunkInfo[id];
    if (fourCC != info.fourCC) {
        throw std::runtime_error(
            "Expected chunk " + std::to_string(id) + " to be '" + fourCCToString(fourCC)
            + "', but is actually '" + fourCCToString(info.fourCC) + "'"
        );
    }

    std::unique_ptr<Common::ReadStream> chunk;
    if (_cachedChunkData.find(id) != _cachedChunkData.end()) {
        auto &data = _cachedChunkData[id];
        chunk = std::make_unique<Common::ReadStream>(data, stream->endianness, 0, data->size());
    } else if (afterburned) {
        stream->seek(info.offset + _ilsBodyOffset);
        unsigned long actualUncompLength = info.uncompressedLen;
        auto chunkStream = stream->readZlibBytes(info.len, &actualUncompLength);
        if (!chunkStream) {
            std::cout << boost::format("Could not uncompress chunk %d\n") % id;
            return nullptr;
        }
        if (info.uncompressedLen != actualUncompLength) {
            throw std::runtime_error(boost::str(
                boost::format("Chunk %d: Expected uncompressed length %d but got length %lu")
                    % id % info.uncompressedLen % actualUncompLength
            ));
        }
        return chunkStream;
    } else {
        stream->seek(info.offset);
        chunk = readChunkData(fourCC, info.len);
    }

    return chunk;
}

std::shared_ptr<Chunk> DirectorFile::readChunk(uint32_t fourCC, uint32_t len) {
    std::unique_ptr<Common::ReadStream> chunkData = readChunkData(fourCC, len);
    return makeChunk(fourCC, *chunkData);
}

std::unique_ptr<Common::ReadStream> DirectorFile::readChunkData(uint32_t fourCC, uint32_t len) {
    auto offset = stream->pos();

    auto validFourCC = stream->readUint32();
    auto validLen = stream->readUint32();

    // use the valid length if mmap hasn't been read yet
    if (len == UINT32_MAX) {
        len = validLen;
    }

    // validate chunk
    if (fourCC != validFourCC || len != validLen) {
        throw std::runtime_error(
            "At offset " + std::to_string(offset)
            + " expected '" + fourCCToString(fourCC) + "' chunk with length " + std::to_string(len)
            + ", but got '" + fourCCToString(validFourCC) + "' chunk with length " + std::to_string(validLen)
        );
    } else {
        std::cout << "At offset " + std::to_string(offset) + " reading chunk '" + fourCCToString(fourCC) + "' with length " + std::to_string(len) + "\n";
    }

    return stream->readBytes(len);
}

std::shared_ptr<Chunk> DirectorFile::makeChunk(uint32_t fourCC, Common::ReadStream &stream) {
    std::shared_ptr<Chunk> res;
    switch (fourCC) {
    case FOURCC('i', 'm', 'a', 'p'):
        res = std::make_shared<InitialMapChunk>(this);
        break;
    case FOURCC('m', 'm', 'a', 'p'):
        res = std::make_shared<MemoryMapChunk>(this);
        break;
    case FOURCC('C', 'A', 'S', '*'):
        res = std::make_shared<CastChunk>(this);
        break;
    case FOURCC('C', 'A', 'S', 't'):
        res = std::make_shared<CastMemberChunk>(this);
        break;
    case FOURCC('K', 'E', 'Y', '*'):
        res = std::make_shared<KeyTableChunk>(this);
        break;
    case FOURCC('L', 'c', 't', 'X'):
        capitalX = true;
        // fall through
    case FOURCC('L', 'c', 't', 'x'):
        res = std::make_shared<ScriptContextChunk>(this);
        break;
    case FOURCC('L', 'n', 'a', 'm'):
        res = std::make_shared<ScriptNamesChunk>(this);
        break;
    case FOURCC('L', 's', 'c', 'r'):
        res = std::make_shared<ScriptChunk>(this);
        break;
    case FOURCC('V', 'W', 'C', 'F'):
    case FOURCC('D', 'R', 'C', 'F'):
        res = std::make_shared<ConfigChunk>(this);
        break;
    case FOURCC('M', 'C', 's', 'L'):
        res = std::make_shared<CastListChunk>(this);
        break;
    default:
        throw std::runtime_error(boost::str(
            boost::format("Could not deserialize '%s' chunk") % fourCCToString(fourCC)
        ));
        break;
    }

    res->read(stream);

    return res;
}

void DirectorFile::dumpScripts() {
    for (const auto &cast : casts) {
        if (!cast->lctx)
            continue;

        for (auto it = cast->lctx->scripts.begin(); it != cast->lctx->scripts.end(); ++it) {
            std::string scriptType;
            std::string id;
            CastMemberChunk *member = it->second->member;
            if (member) {
                if (member->type == kScriptMember) {
                    ScriptMember *scriptMember = static_cast<ScriptMember *>(member->member.get());
                    switch (scriptMember->scriptType) {
                    case kScoreScript:
                        scriptType = (version >= 600) ? "BehaviorScript" : "ScoreScript";
                        break;
                    case kMovieScript:
                        scriptType = "MovieScript";
                        break;
                    case kParentScript:
                        scriptType = "ParentScript";
                        break;
                    default:
                        scriptType = "UnknownScript";
                        break;
                    }
                } else {
                    scriptType = "CastScript";
                }
                id = member->info->name.empty()
                    ? std::to_string(member->id)
                    : member->info->name;
            } else {
                scriptType = "UnknownScript";
                id = std::to_string(it->first);
            }
            std::string fileName = cleanFileName("Cast " + cast->name + " " + scriptType + " " + id);
            Common::writeFile(fileName + ".ls", it->second->scriptText());
            Common::writeFile(fileName + ".lasm", it->second->bytecodeText());
        }
    }
}

void DirectorFile::dumpChunks() {
    for (auto it = chunkInfo.begin(); it != chunkInfo.end(); it++) {
        const auto &info = it->second;
        if (info.offset == 0) // RIFX
            continue;

        std::string fileName = cleanFileName(fourCCToString(info.fourCC) + "-" + std::to_string(info.id));
        std::shared_ptr<Common::ReadStream> chunk = getChunkData(info.fourCC, info.id);
        if (chunk) {
            Common::writeFile(fileName + ".bin", chunk->getData(), chunk->len());
        }
        if (deserializedChunks.find(info.id) != deserializedChunks.end()) {
            ordered_json j = *deserializedChunks[info.id];
            std::stringstream ss;
            ss << j.dump(4) << std::endl;
            Common::writeFile(fileName + ".json", ss.str());
        }
    }
}

}
