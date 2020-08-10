#ifndef MOVIE_H
#define MOVIE_H

#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ProjectorRays {

struct Chunk;
// struct Cast;
struct ReadStream;

struct Movie {
    ReadStream *stream;
    std::shared_ptr<MetaChunk> meta;
    std::shared_ptr<InitialMapChunk> imap;
    std::shared_ptr<MemoryMapChunk> mmap;

    int version;
    bool capitalX;

    std::map<uint32_t, std::shared_ptr<Chunk>> chunkMap;
    // std::map<uint32_t, std::shared_ptr<Cast>> castMap;
    std::vector<std::shared_ptr<ScriptContextChunk>> scriptContexts;

    Movie() : version(0), capitalX(false) {}

    void read(ReadStream *s);
    void lookupMmap();
    bool readConfig();
    bool readCasts();
    void readScripts();
    std::shared_ptr<Chunk> getChunk(uint32_t fourCC, uint32_t offset);
    std::shared_ptr<Chunk> readChunk(uint32_t fourCC, uint32_t len = UINT32_MAX);
};

}

#endif
