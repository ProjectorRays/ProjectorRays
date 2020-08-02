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

    bool capitalX;

    std::map<uint32_t, std::shared_ptr<Chunk>> chunkMap;
    // std::map<uint32_t, std::shared_ptr<Cast>> castMap;
    std::vector<std::shared_ptr<ScriptContextChunk>> scriptContexts;

    Movie() : capitalX(false) {}

    void read(ReadStream *s);
    void lookupMmap();
    // void createCasts();
    void readScripts();
    std::shared_ptr<Chunk> getChunk(uint32_t fourCC, uint32_t offset);
    std::shared_ptr<Chunk> readChunk(uint32_t fourCC, uint32_t len = UINT32_MAX);
};

}

#endif
