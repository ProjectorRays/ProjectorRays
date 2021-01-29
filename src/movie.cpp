#include <iostream>
#include <stdexcept>

#include "chunk.h"
#include "lingo.h"
#include "movie.h"
#include "stream.h"
#include "subchunk.h"
#include "util.h"

namespace ProjectorRays {

/* Movie */

void Movie::read(ReadStream *s) {
    stream = s;
    stream->endianness = kBigEndian; // we set this properly when we create the RIFX chunk
    readMemoryMap();
    readKeyTable();
    readConfig();
    readCasts();
}

void Movie::readMemoryMap() {
    // at the beginning of the file, we need to break some of the typical rules. We don't know names, lengths and offsets yet.

    // Meta
    std::shared_ptr<MetaChunk> meta = std::static_pointer_cast<MetaChunk>(readChunk(FOURCC('R', 'I', 'F', 'X')));

    if (meta->codec != FOURCC('M', 'V', '9', '3')) {
        throw std::runtime_error("Codec unsupported: " + fourCCToString(meta->codec));
    }

    // Initial map
    std::shared_ptr<InitialMapChunk> imap = std::static_pointer_cast<InitialMapChunk>(readChunk(FOURCC('i', 'm', 'a', 'p')));

    // Memory map
    stream->seek(imap->memoryMapOffset);
    std::shared_ptr<MemoryMapChunk> mmap = std::static_pointer_cast<MemoryMapChunk>(readChunk(FOURCC('m', 'm', 'a', 'p')));

    for (uint32_t i = 0; i < mmap->mapArray.size(); i++) {
        auto mapEntry = mmap->mapArray[i];

        if (mapEntry.fourCC == FOURCC('f', 'r', 'e', 'e') || mapEntry.fourCC == FOURCC('j', 'u', 'n', 'k'))
            continue;
        
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

bool Movie::readKeyTable() {
    auto info = getFirstChunkInfo(FOURCC('K', 'E', 'Y', '*'));
    if (info) {
        keyTable = std::static_pointer_cast<KeyTableChunk>(getChunk(info->fourCC, info->id));
        return true;
    }

    std::cout << "No key chunk!\n";
    return false;
}

bool Movie::readConfig() {
    auto info = getFirstChunkInfo(FOURCC('V', 'W', 'C', 'F'));
    if (!info)
        info = getFirstChunkInfo(FOURCC('D', 'R', 'C', 'F'));

    if (info) {
        config = std::static_pointer_cast<ConfigChunk>(getChunk(info->fourCC, info->id));
        version = humanVersion(config->directorVersion);
        std::cout << "Director version: " + std::to_string(version) + "\n";

        return true;
    }

    std::cout << "No config chunk!\n";
    return false;
}

bool Movie::readCasts() {
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
        }

        std::cout << "No cast list!\n";
        return false;
    } else {
        auto info = getFirstChunkInfo(FOURCC('C', 'A', 'S', '*'));
        if (info) {
            auto cast = std::static_pointer_cast<CastChunk>(getChunk(info->fourCC, info->id));
            cast->populate("Internal", 1024, config->minMember);
            casts.push_back(std::move(cast));

            return true;
        }

        std::cout << "No cast!\n";
        return false;
    }

    return false;
}

const ChunkInfo *Movie::getFirstChunkInfo(uint32_t fourCC) {
    auto &chunkIDs = chunkIDsByFourCC[fourCC];
    if (chunkIDs.size() > 0) {
        return &chunkInfo[chunkIDs[0]];
    }
    return nullptr;
}

std::shared_ptr<Chunk> Movie::getChunk(uint32_t fourCC, int32_t id) {
    if (deserializedChunks.find(id) != deserializedChunks.end())
        return deserializedChunks[id];

    if (chunkInfo.find(id) == chunkInfo.end())
        throw std::runtime_error("Could not find chunk " + std::to_string(id));

    auto &info = chunkInfo[id];
    if (fourCC != info.fourCC) {
        throw std::runtime_error(
            "Expected chunk " + std::to_string(id) + " to be '" + fourCCToString(fourCC)
            + "', but is actually '" + fourCCToString(info.fourCC) + "'"
        );
    }

    stream->seek(info.offset);
    std::shared_ptr<Chunk> chunk = readChunk(fourCC, info.len);

    // don't cache the deserialized map chunks
    // we'll just generate a new one if we need to save
    if (fourCC != FOURCC('R', 'I', 'F', 'X') && fourCC != FOURCC('i', 'm', 'a', 'p') && fourCC != FOURCC('m', 'm', 'a', 'p')) {
        deserializedChunks[id] = chunk;
    }

    return chunk;
}

std::shared_ptr<Chunk> Movie::readChunk(uint32_t fourCC, uint32_t len) {
    auto offset = stream->pos();

    // check if this is the chunk we are expecting
    auto validFourCC = stream->readUint32();
    if (fourCC == FOURCC('R', 'I', 'F', 'X')) {
        //if (validName.substring(0, 2) == "MZ") {
            // handle Projector HERE
        //}
        if (validFourCC == FOURCC('X', 'F', 'I', 'R')) {
            stream->endianness = kLittleEndian;
            validFourCC = FOURCC('R', 'I', 'F', 'X');
        }
    }
    // check if it has the length the mmap table specifies
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

    if (fourCC == FOURCC('R', 'I', 'F', 'X')) {
        // we're going to pretend RIFX is only 12 bytes long
        // this is because offsets are relative to the beginning of the file
        // whereas everywhere else they're relative to chunk start
        len = 4;
    }

    // copy the contents of the chunk to a new DataStream (minus name/length as that's not what offsets are usually relative to)
    auto chunkStream = stream->readBytes(len);

    std::shared_ptr<Chunk> res;
    switch (fourCC) {
    case FOURCC('R', 'I', 'F', 'X'):
        res = std::make_shared<MetaChunk>(this);
        break;
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
        res = std::make_shared<Chunk>(this);
    }

    res->read(*chunkStream);

    return res;
}

}
