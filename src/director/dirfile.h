/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_DIRFILE_H
#define DIRECTOR_DIRFILE_H

#include <cstdint>
#include <istream>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common/stream.h"
#include "director/guid.h"

namespace Director {

struct Chunk;
struct CastChunk;
struct ConfigChunk;
struct KeyTableChunk;
struct InitialMapChunk;
struct MemoryMapChunk;

struct ChunkInfo {
	int32_t id;
	uint32_t fourCC;
	uint32_t len;
	uint32_t uncompressedLen;
	int32_t offset;
	MoaID compressionID;
};

class DirectorFile {
private:
	size_t _ilsBodyOffset;
	std::vector<uint8_t> _ilsBuf;

	std::map<int32_t, std::vector<uint8_t>> _cachedChunkBufs;
	std::map<int32_t, Common::BufferView> _cachedChunkViews;

public:
	Common::ReadStream *stream;
	KeyTableChunk *keyTable;
	ConfigChunk *config;

	Common::Endianness endianness;
	std::string fverVersionString;
	unsigned int version;
	bool capitalX;
	bool dotSyntax;
	uint32_t codec;
	bool afterburned;

	std::map<uint32_t, std::vector<int32_t>> chunkIDsByFourCC;
	std::map<int32_t, ChunkInfo> chunkInfo;
	std::map<int32_t, std::shared_ptr<Chunk>> deserializedChunks;

	std::vector<CastChunk *> casts;

	std::unique_ptr<InitialMapChunk> initialMap;
	std::unique_ptr<MemoryMapChunk> memoryMap;

	DirectorFile();
	~DirectorFile();

	bool read(Common::ReadStream *s);
	void readMemoryMap();
	bool readAfterburnerMap();
	bool readKeyTable();
	bool readConfig();
	bool readCasts();
	const ChunkInfo *getFirstChunkInfo(uint32_t fourCC);
	bool chunkExists(uint32_t fourCC, int32_t id);
	Chunk *getChunk(uint32_t fourCC, int32_t id);
	Common::BufferView getChunkData(uint32_t fourCC, int32_t id);
	std::shared_ptr<Chunk> readChunk(uint32_t fourCC, uint32_t len = UINT32_MAX);
	Common::BufferView readChunkData(uint32_t fourCC, uint32_t len);
	std::shared_ptr<Chunk> makeChunk(uint32_t fourCC, const Common::BufferView &view);

	bool compressionImplemented(MoaID compressionID);

	size_t size();
	size_t chunkSize(int32_t id);

	void writeToFile(const std::filesystem::path &path);
	void generateInitialMap();
	void generateMemoryMap();
	void write(Common::WriteStream &stream);
	void writeChunk(Common::WriteStream &stream, int32_t id);

	void parseScripts();
	void restoreScriptText();

	void dumpScripts(std::filesystem::path castsDir);
	void dumpChunks(std::filesystem::path chunksDir);
	void dumpJSON(std::filesystem::path chunksDir);

	bool isCast() const;
};

} // namespace Director

#endif // DIRECTOR_DIRFILE_H
