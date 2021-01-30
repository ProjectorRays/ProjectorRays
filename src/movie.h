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

struct ChunkInfo {
    int32_t id;
    uint32_t fourCC;
    int32_t len;
    int32_t uncompressedLen;
    int32_t offset;
    int32_t compressionType;
};

class Movie {
private:
    std::map<int32_t, std::shared_ptr<std::vector<uint8_t>>> _cachedChunkData;

public:
    ReadStream *stream;
    std::shared_ptr<KeyTableChunk> keyTable;
    std::shared_ptr<ConfigChunk> config;

    int version;
    bool capitalX;
    uint32_t codec;
    bool afterburned;

    std::map<uint32_t, std::vector<int32_t>> chunkIDsByFourCC;
    std::map<int32_t, ChunkInfo> chunkInfo;
    std::map<int32_t, std::shared_ptr<Chunk>> deserializedChunks;

    std::vector<std::shared_ptr<CastChunk>> casts;

    Movie() : stream(nullptr), version(0), capitalX(false), codec(0), afterburned(false) {}

    void read(ReadStream *s);
    void readMemoryMap();
    bool readKeyTable();
    bool readConfig();
    bool readCasts();
    const ChunkInfo *getFirstChunkInfo(uint32_t fourCC);
    std::shared_ptr<Chunk> getChunk(uint32_t fourCC, int32_t offset);
    std::shared_ptr<Chunk> readChunk(uint32_t fourCC, uint32_t len = UINT32_MAX);
};

}

#endif
