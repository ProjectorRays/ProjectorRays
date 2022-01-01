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

#include <sstream>
#include <stdexcept>

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "common/fileio.h"
#include "common/log.h"
#include "common/stream.h"
#include "director/chunk.h"
#include "director/lingo.h"
#include "director/dirfile.h"
#include "director/subchunk.h"
#include "director/util.h"

namespace Director {

/* DirectorFile */

void DirectorFile::read(Common::ReadStream *s, bool decompile) {
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
	if (codec == FOURCC('M', 'V', '9', '3') || codec == FOURCC('M', 'C', '9', '5')) {
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
	if (decompile) {
		if (!readCasts())
			return;
	}
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

		Common::debug(boost::format("Found RIFX resource index %d: '%s', %d bytes @ pos 0x%08x (%d)")
						% i % fourCCToString(mapEntry.fourCC) % mapEntry.len % mapEntry.offset % mapEntry.offset);

		ChunkInfo info;
		info.id = i;
		info.fourCC = mapEntry.fourCC;
		info.len = mapEntry.len;
		info.uncompressedLen = mapEntry.len;
		info.offset = mapEntry.offset;
		info.compressionType = -1;
		chunkInfo[i] = info;

		chunkIDsByFourCC[mapEntry.fourCC].push_back(i);
	}
}

bool DirectorFile::readAfterburnerMap() {
	uint32_t start, end;

	// File version
	if (stream->readUint32() != FOURCC('F', 'v', 'e', 'r')) {
		Common::log("readAfterburnerMap(): Fver expected but not found");
		return false;
	}

	uint32_t fverLength = stream->readVarInt();
	start = stream->pos();
	uint32_t version = stream->readVarInt();
	Common::debug(boost::format("Fver: version: %x") % version);
	end = stream->pos();

	if (end - start != fverLength) {
		Common::log(boost::format("readAfterburnerMap(): Expected Fver of length %d but read %d bytes")
						% fverLength % (end - start));
		stream->seek(start + fverLength);
	}

	// Compression types
	if (stream->readUint32() != FOURCC('F', 'c', 'd', 'r')) {
		Common::log("readAfterburnerMap(): Fcdr expected but not found");
		return false;
	}

	uint32_t fcdrLength = stream->readVarInt();
	stream->skip(fcdrLength);

	// Afterburner map
	if (stream->readUint32() != FOURCC('A', 'B', 'M', 'P')) {
		Common::log("RIFXArchive::readAfterburnerMap(): ABMP expected but not found");
		return false;
	}
	uint32_t abmpLength = stream->readVarInt();
	uint32_t abmpEnd = stream->pos() + abmpLength;
	int32_t abmpCompressionType = stream->readVarInt();
	unsigned long abmpUncompLength = stream->readVarInt();
	unsigned long abmpActualUncompLength = abmpUncompLength;
	Common::debug(boost::format("ABMP: length: %d compressionType: %d uncompressedLength: %lu")
					% abmpLength % abmpCompressionType % abmpUncompLength);

	auto abmpStream = stream->readZlibBytes(abmpEnd - stream->pos(), &abmpActualUncompLength);
	if (!abmpStream) {
		Common::log("RIFXArchive::readAfterburnerMap(): Could not uncompress ABMP");
		return false;
	}
	if (abmpUncompLength != abmpActualUncompLength) {
		Common::log(boost::format("ABMP: Expected uncompressed length %lu but got length %lu")
						% abmpUncompLength % abmpActualUncompLength);
	}

	uint32_t abmpUnk1 = abmpStream->readVarInt();
	uint32_t abmpUnk2 = abmpStream->readVarInt();
	uint32_t resCount = abmpStream->readVarInt();
	Common::debug(boost::format("ABMP: unk1: %d unk2: %d resCount: %d")
					% abmpUnk1 % abmpUnk2 % resCount);

	for (uint32_t i = 0; i < resCount; i++) {
		uint32_t resId = abmpStream->readVarInt();
		int32_t offset = abmpStream->readVarInt();
		uint32_t compSize = abmpStream->readVarInt();
		uint32_t uncompSize = abmpStream->readVarInt();
		int32_t compressionType = abmpStream->readVarInt();
		uint32_t tag = abmpStream->readUint32();

		Common::debug(boost::format("Found RIFX resource index %d: '%s', %d bytes (%d uncompressed) @ pos 0x%08x (%d), compressionType: %d")
						% resId % fourCCToString(tag) % compSize % uncompSize % offset % offset % compressionType);

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
		Common::log("readAfterburnerMap(): Map has no entry for ILS");
		return false;
	}
	if (stream->readUint32() != FOURCC('F', 'G', 'E', 'I')) {
		Common::log("readAfterburnerMap(): FGEI expected but not found");
		return false;
	}

	ChunkInfo &ilsInfo = chunkInfo[2];
	uint32_t ilsUnk1 = stream->readVarInt();
	Common::debug(boost::format("ILS: length: %d unk1: %d") % ilsInfo.len % ilsUnk1);
	_ilsBodyOffset = stream->pos();
	unsigned long ilsActualUncompLength = ilsInfo.uncompressedLen;
	auto ilsStream = stream->readZlibBytes(ilsInfo.len, &ilsActualUncompLength);
	if (!ilsStream) {
		Common::log("readAfterburnerMap(): Could not uncompress FGEI");
		return false;
	}
	if (ilsInfo.uncompressedLen != ilsActualUncompLength) {
		Common::log(boost::format("ILS: Expected uncompressed length %d but got length %lu")
						% ilsInfo.uncompressedLen % ilsActualUncompLength);
	}

	while (!ilsStream->eof()) {
		uint32_t resId = ilsStream->readVarInt();
		ChunkInfo &info = chunkInfo[resId];

		Common::debug(boost::format("Loading ILS resource %d: '%s', %d bytes")
						% resId % fourCCToString(info.fourCC) % info.len);

		auto data = ilsStream->copyBytes(info.len);
		if (data) {
			_cachedChunkData[resId] = std::move(data);
		} else {
			Common::log(boost::format("Could not load ILS resource %d") % resId);
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
			Common::debug(boost::format("KEY* entry %d: '%s' @ %d owned by '%s' @ %d")
				% i % fourCCToString(entry.fourCC) % entry.sectionID % fourCCToString(ownerTag) % entry.castID);
		}

		return true;
	}

	Common::log("No key chunk!");
	return false;
}

bool DirectorFile::readConfig() {
	auto info = getFirstChunkInfo(FOURCC('V', 'W', 'C', 'F'));
	if (!info)
		info = getFirstChunkInfo(FOURCC('D', 'R', 'C', 'F'));

	if (info) {
		config = std::static_pointer_cast<ConfigChunk>(getChunk(info->fourCC, info->id));
		version = humanVersion(config->directorVersion);
		Common::log("Director version: " + std::to_string(version));
		dotSyntax = (version >= 700); // TODO: Check for verbose/dot syntax opcodes, allow users to toggle this

		return true;
	}

	Common::log("No config chunk!");
	return false;
}

bool DirectorFile::readCasts() {
	bool internal = true;

	if (version >= 500) {
		auto info = getFirstChunkInfo(FOURCC('M', 'C', 's', 'L'));
		if (info) {
			auto castList = std::static_pointer_cast<CastListChunk>(getChunk(info->fourCC, info->id));
			for (const auto &castEntry : castList->entries) {
				Common::debug("Cast: " + castEntry.name);
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

	Common::log("No cast!");
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
			Common::log(boost::format("Could not uncompress chunk %d") % id);
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
		Common::debug("At offset " + std::to_string(offset) + " reading chunk '" + fourCCToString(fourCC) + "' with length " + std::to_string(len));
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
	}
}

void DirectorFile::dumpJSON() {
	for (auto it = chunkInfo.begin(); it != chunkInfo.end(); it++) {
		const auto &info = it->second;
		if (info.offset == 0) // RIFX
			continue;

		std::string fileName = cleanFileName(fourCCToString(info.fourCC) + "-" + std::to_string(info.id));
		if (deserializedChunks.find(info.id) != deserializedChunks.end()) {
			ordered_json j = *deserializedChunks[info.id];
			std::stringstream ss;
			ss << j.dump(4) << std::endl;
			Common::writeFile(fileName + ".json", ss.str());
		}
	}
}

}
