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

void Movie::read(ReadStream &stream) {
    stream.endianness = kBigEndian; // we set this properly when we create the RIFX chunk
    lookupMmap(stream);
    // createCasts();
    linkScripts();
}

void Movie::lookupMmap(ReadStream &stream) {
    // at the beginning of the file, we need to break some of the typical rules. We don't know names, lengths and offsets yet.

    // valid length is undefined because we have not yet reached mmap
    // however, it will be filled automatically in chunk's constructor
    auto meta = std::static_pointer_cast<MetaChunk>(readChunk(stream, FOURCC('R', 'I', 'F', 'X')));
    // we can only open DIR or DXR
    // we'll read Movie from stream because Movie is an exception to the normal rules
    if (meta->codec != FOURCC('M', 'V', '9', '3')) {
        throw std::runtime_error("Codec unsupported: " + fourCCToString(meta->codec));
    }

    // the next chunk should be imap
    auto imap = std::static_pointer_cast<InitialMapChunk>(readChunk(stream, FOURCC('i', 'm', 'a', 'p')));

    stream.seek(imap->memoryMapOffset);
    auto mmap = std::static_pointer_cast<MemoryMapChunk>(readChunk(stream, FOURCC('m', 'm', 'a', 'p')));
    // add chunks in the mmap to the chunkArrays HERE
    for (uint32_t i = 0; i < mmap->mapArray.size(); i++) {
        auto mapEntry = mmap->mapArray[i];

        if (mapEntry.fourCC == FOURCC('f', 'r', 'e', 'e') || mapEntry.fourCC == FOURCC('j', 'u', 'n', 'k'))
            continue;

        std::shared_ptr<Chunk> chunk;
        if (mapEntry.fourCC == FOURCC('R', 'I', 'F', 'X')) {
            chunk = meta;
        } else if (mapEntry.fourCC == FOURCC('i', 'm', 'a', 'p')) {
            chunk = imap;
        } else if (mapEntry.fourCC == FOURCC('m', 'm', 'a', 'p')) {
            chunk = mmap;
        } else {
            stream.seek(mapEntry.offset);
            chunk = readChunk(stream, mapEntry.fourCC, mapEntry.len);
        }
        chunkArrays[mapEntry.fourCC].push_back(chunk);
        chunkMap[i] = chunk;
    }
}

// void Movie::createCasts() {
//     let castList = this.chunkArrays.get("MCsL")[0] as Chunk.CastList;
//     let castKey = this.chunkArrays.get("KEY*")[0] as Chunk.CastKey;
//     for (let entry of castList.entries) {
//         let cast = new Cast();
//         cast.readDataEntry(entry);
//         cast.readKey(castKey, this.chunkMap);
//         this.castMap[cast.id] = cast;
//     }
// }

void Movie::linkScripts() {
    auto ctxArray = chunkArrays[FOURCC('L', 'c', 't', 'X')];
    for (size_t i = 0; i < ctxArray.size(); i ++) {
        auto context = std::dynamic_pointer_cast<ScriptContextChunk>(ctxArray[i]);
        auto scriptNames = std::dynamic_pointer_cast<ScriptNamesChunk>(chunkMap[context->lnamSectionID]);
        for (size_t j = 0; j < context->sectionMap.size(); j++) {
            auto section = context->sectionMap[j];
            if (section.sectionID > -1) {
                auto script = std::dynamic_pointer_cast<ScriptChunk>(chunkMap[section.sectionID]);
                script->readNames(scriptNames->names);
                script->translate(scriptNames->names);
                context->scripts.push_back(script);
            }
        }
    }
}

std::shared_ptr<Chunk> Movie::readChunk(ReadStream &stream, uint32_t fourCC, uint32_t len) {
    auto offset = stream.pos();

    // check if this is the chunk we are expecting
    auto validFourCC = stream.readUint32();
    if (fourCC == FOURCC('R', 'I', 'F', 'X')) {
        //if (validName.substring(0, 2) == "MZ") {
            // handle Projector HERE
        //}
        if (validFourCC == FOURCC('X', 'F', 'I', 'R')) {
            stream.endianness = kLittleEndian;
            validFourCC = FOURCC('R', 'I', 'F', 'X');
        }
    }
    // check if it has the length the mmap table specifies
    auto validLen = stream.readUint32();

    // use the valid length if mmap hasn't been read yet
    if (len == UINT32_MAX) {
        len = validLen;
    }

    // padding can't be checked, so let's give it a default value if we don't yet know it
    // if (padding == null) {
    //     // padding is usually zero
    //     if (name !== "free" && name !== "junk") {
    //         padding = 0;
    //     } else {
    //         padding = 12;
    //     }
    // }

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
    auto chunkStream = stream.readBytes(len);

    std::shared_ptr<Chunk> res;
    switch (fourCC) {
    case FOURCC('R', 'I', 'F', 'X'):
        res = std::make_shared<MetaChunk>();
        break;
    case FOURCC('i', 'm', 'a', 'p'):
        res = std::make_shared<InitialMapChunk>();
        break;
    case FOURCC('m', 'm', 'a', 'p'):
        res = std::make_shared<MemoryMapChunk>();
        break;
    case FOURCC('L', 'c', 't', 'X'):
        res = std::make_shared<ScriptContextChunk>();
        break;
    case FOURCC('L', 'n', 'a', 'm'):
        res = std::make_shared<ScriptNamesChunk>();
        break;
    case FOURCC('L', 's', 'c', 'r'):
        res = std::make_shared<ScriptChunk>();
        break;
    case FOURCC('M', 'C', 's', 'L'):
        res = std::make_shared<CastListChunk>();
        break;
    case FOURCC('K', 'E', 'Y', '*'):
        res = std::make_shared<CastKeyChunk>();
        break;
    case FOURCC('C', 'A', 'S', '*'):
        res = std::make_shared<CastAssociationsChunk>();
        break;
    case FOURCC('C', 'A', 'S', 't'):
        res = std::make_shared<CastMemberChunk>();
        break;
    default:
        res = std::make_shared<Chunk>(fourCC);
    }

    res->read(*chunkStream);

    return res;
}

}
