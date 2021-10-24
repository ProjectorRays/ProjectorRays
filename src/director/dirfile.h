/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2021 Deborah Servilla
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef DIRECTOR_DIRFILE_H
#define DIRECTOR_DIRFILE_H

#include <cstdint>
#include <istream>
#include <map>
#include <memory>
#include <string>
#include <vector>


namespace Common {
class ReadStream;
}

namespace Director {

struct Chunk;
struct CastChunk;
struct ConfigChunk;
struct KeyTableChunk;

struct ChunkInfo {
	int32_t id;
	uint32_t fourCC;
	uint32_t len;
	uint32_t uncompressedLen;
	int32_t offset;
	uint32_t compressionType;
};

class DirectorFile {
private:
	std::map<int32_t, std::shared_ptr<std::vector<uint8_t>>> _cachedChunkData;
	size_t _ilsBodyOffset;

public:
	Common::ReadStream *stream;
	std::shared_ptr<KeyTableChunk> keyTable;
	std::shared_ptr<ConfigChunk> config;

	int version;
	bool capitalX;
	bool dotSyntax;
	uint32_t codec;
	bool afterburned;

	std::map<uint32_t, std::vector<int32_t>> chunkIDsByFourCC;
	std::map<int32_t, ChunkInfo> chunkInfo;
	std::map<int32_t, std::shared_ptr<Chunk>> deserializedChunks;

	std::vector<std::shared_ptr<CastChunk>> casts;

	DirectorFile() : _ilsBodyOffset(0), stream(nullptr), version(0), capitalX(false), codec(0), afterburned(false) {}

	void read(Common::ReadStream *s, bool decompile = true);
	void readMemoryMap();
	bool readAfterburnerMap();
	bool readKeyTable();
	bool readConfig();
	bool readCasts();
	const ChunkInfo *getFirstChunkInfo(uint32_t fourCC);
	bool chunkExists(uint32_t fourCC, int32_t id);
	std::shared_ptr<Chunk> getChunk(uint32_t fourCC, int32_t id);
	std::unique_ptr<Common::ReadStream> getChunkData(uint32_t fourCC, int32_t id);
	std::shared_ptr<Chunk> readChunk(uint32_t fourCC, uint32_t len = UINT32_MAX);
	std::unique_ptr<Common::ReadStream> readChunkData(uint32_t fourCC, uint32_t len);
	std::shared_ptr<Chunk> makeChunk(uint32_t fourCC, Common::ReadStream &stream);

	void dumpScripts();
	void dumpChunks();
};

}

#endif
