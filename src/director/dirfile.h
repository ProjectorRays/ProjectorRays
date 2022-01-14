/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2022 Deborah Servilla
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DIRECTOR_DIRFILE_H
#define DIRECTOR_DIRFILE_H

#include <cstdint>
#include <istream>
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
	std::shared_ptr<KeyTableChunk> keyTable;
	std::shared_ptr<ConfigChunk> config;

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

	std::vector<std::shared_ptr<CastChunk>> casts;

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
	std::shared_ptr<Chunk> getChunk(uint32_t fourCC, int32_t id);
	Common::BufferView getChunkData(uint32_t fourCC, int32_t id);
	std::shared_ptr<Chunk> readChunk(uint32_t fourCC, uint32_t len = UINT32_MAX);
	Common::BufferView readChunkData(uint32_t fourCC, uint32_t len);
	std::shared_ptr<Chunk> makeChunk(uint32_t fourCC, const Common::BufferView &view);

	bool compressionImplemented(MoaID compressionID);

	size_t size();
	size_t chunkSize(int32_t id);

	void writeToFile(std::string fileName);
	void generateInitialMap();
	void generateMemoryMap();
	void write(Common::WriteStream &stream);
	void writeChunk(Common::WriteStream &stream, int32_t id);

	void restoreScriptText();

	void dumpScripts();
	void dumpChunks();
	void dumpJSON();
};

}

#endif
