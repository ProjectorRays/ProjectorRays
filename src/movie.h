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
struct Cast;
struct ReadStream;

struct Movie {
    std::map<uint32_t, std::vector<std::shared_ptr<Chunk>>> chunkArrays;
    std::map<uint32_t, std::shared_ptr<Chunk>> chunkMap;
    std::map<uint32_t, std::shared_ptr<Cast>> castMap;

    void read(ReadStream &stream);
    void lookupMmap(ReadStream &stream);
    // void createCasts();
    void linkScripts();
    std::shared_ptr<Chunk> readChunk(ReadStream &stream, uint32_t fourCC, uint32_t len = UINT32_MAX);
};

}

#endif
