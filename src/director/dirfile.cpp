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

static const size_t kRIFXHeaderSize = 12;

/* DirectorFile */

DirectorFile::DirectorFile() :
	_ilsBodyOffset(0),
	stream(nullptr),
	version(0),
	capitalX(false),
	codec(0),
	afterburned(false) {}

DirectorFile::~DirectorFile() = default;

// read stuff

void DirectorFile::read(Common::ReadStream *s, bool decompile) {
	stream = s;
	stream->endianness = Common::kBigEndian; // we set this properly when we create the RIFX chunk

	// Meta
	auto metaFourCC = stream->readUint32();
	if (metaFourCC == FOURCC('X', 'F', 'I', 'R')) {
		stream->endianness = Common::kLittleEndian;
	}
	endianness = stream->endianness;
	stream->readUint32(); // meta length
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
		Common::log(boost::format("readAfterburnerMap(): Expected Fver of length %u but read %u bytes")
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
	Common::debug(boost::format("ABMP: length: %u compressionType: %u uncompressedLength: %lu")
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
	Common::debug(boost::format("ABMP: unk1: %u unk2: %u resCount: %u")
					% abmpUnk1 % abmpUnk2 % resCount);

	for (uint32_t i = 0; i < resCount; i++) {
		int32_t resId = abmpStream->readVarInt();
		int32_t offset = abmpStream->readVarInt();
		uint32_t compSize = abmpStream->readVarInt();
		uint32_t uncompSize = abmpStream->readVarInt();
		int32_t compressionType = abmpStream->readVarInt();
		uint32_t tag = abmpStream->readUint32();

		Common::debug(boost::format("Found RIFX resource index %d: '%s', %u bytes (%u uncompressed) @ pos 0x%08x (%d), compressionType: %d")
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
		Common::log(boost::format("ILS: Expected uncompressed length %u but got length %lu")
						% ilsInfo.uncompressedLen % ilsActualUncompLength);
	}

	while (!ilsStream->eof()) {
		int32_t resId = ilsStream->readVarInt();
		ChunkInfo &info = chunkInfo[resId];

		Common::debug(boost::format("Loading ILS resource %d: '%s', %u bytes")
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
			Common::debug(boost::format("KEY* entry %u: '%s' @ %d owned by '%s' @ %d")
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
		if (info.compressionType == 0) {
			// Chunk is zlib compressed
			unsigned long actualUncompLength = info.uncompressedLen;
			auto chunkStream = stream->readZlibBytes(info.len, &actualUncompLength);
			if (!chunkStream) {
				Common::log(boost::format("Could not uncompress '%s' %d") % fourCCToString(fourCC) % id);
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
			// Stuff like 'snd '
			return stream->readBytes(info.len);
		}
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

// write stuff

void DirectorFile::writeToFile(std::string fileName) {
	generateInitialMap();
	generateMemoryMap();
	auto data = std::make_shared<std::vector<uint8_t>>(size());
	auto stream = std::make_unique<Common::WriteStream>(data);
	write(*stream);
	Common::writeFile(fileName, data->data(), data->size());
}

void DirectorFile::generateInitialMap() {
	initialMap = std::make_unique<InitialMapChunk>(this);
	initialMap->one = 1;
	initialMap->mmapOffset = kRIFXHeaderSize + initialMap->size();
	initialMap->version = (version < 500) ? 0 : config->directorVersion;
	initialMap->unused1 = 0;
	initialMap->unused2 = 0;
	initialMap->unused3 = 0;
}

void DirectorFile::generateMemoryMap() {
	// Figure out how many slots we'll need
	int32_t maxID = 2; // the mmap's ID
	for (auto [id, info] : chunkInfo) {
		if (id > maxID) {
			maxID = id;
		}
	}

	memoryMap = std::make_unique<MemoryMapChunk>(this);
	memoryMap->headerLength = 24;
	memoryMap->entryLength = 20;
	memoryMap->chunkCountMax = maxID + 1;
	memoryMap->chunkCountUsed = maxID + 1;
	memoryMap->freeHead = -1;
	memoryMap->junkHead = -1;
	memoryMap->junkHead = -1;

	// Fill the map with free entries
	memoryMap->mapArray.resize(maxID + 1);
	for (auto &entry : memoryMap->mapArray) {
		entry.fourCC = FOURCC('f', 'r', 'e', 'e');
		entry.len = 0;
		entry.offset = 0;
		entry.flags = 12;
		entry.unknown0 = 0;
		entry.next = 0;
	}

	// Fill in the actual entries
	int32_t nextOffset = 0;

	auto &rifxEntry = memoryMap->mapArray[0];
	rifxEntry.fourCC = FOURCC('R', 'I', 'F', 'X');
	// length to be calculated...
	rifxEntry.offset = nextOffset;
	rifxEntry.flags = 1;
	rifxEntry.unknown0 = 0;
	rifxEntry.next = 0;
	nextOffset += kRIFXHeaderSize;

	auto &imapEntry = memoryMap->mapArray[1];
	imapEntry.fourCC = FOURCC('i', 'm', 'a', 'p');
	imapEntry.len = initialMap->size();
	imapEntry.offset = nextOffset;
	imapEntry.flags = 0;
	imapEntry.unknown0 = 0;
	imapEntry.next = 0;
	nextOffset += imapEntry.len;

	auto &mmapEntry = memoryMap->mapArray[2];
	mmapEntry.fourCC = FOURCC('m', 'm', 'a', 'p');
	mmapEntry.len = memoryMap->size();
	mmapEntry.offset = nextOffset;
	mmapEntry.flags = 0;
	mmapEntry.unknown0 = 0;
	mmapEntry.next = 0;
	nextOffset += mmapEntry.len;

	for (auto [id, info] : chunkInfo) {
		if (id <= 2) // Ignore RIFX, imap, mmap
			continue;

		auto &entry = memoryMap->mapArray[id];
		entry.fourCC = info.fourCC;
		entry.len = chunkSize(id);
		entry.offset = nextOffset;
		entry.flags = 0;
		entry.unknown0 = 0;
		entry.next = 0;
		nextOffset += 8 + entry.len;
	}

	rifxEntry.len = nextOffset;

	// Now link the free entries
	for (int32_t id = maxID; id >= 0; id--) {
		auto &entry = memoryMap->mapArray[id];
		if (entry.fourCC == FOURCC('f', 'r', 'e', 'e')) {
			entry.next = memoryMap->freeHead;
			memoryMap->freeHead = id;
		}
	}
}

size_t DirectorFile::size() {
	return memoryMap->mapArray[0].len;
}

size_t DirectorFile::chunkSize(int32_t id) {
	// If we've implemented writing for this chunk, recalculate its size.
	if (deserializedChunks.find(id) != deserializedChunks.end()) {
		Chunk &chunk = *deserializedChunks[id];
		if (chunk.writable) {
			return chunk.size();
		}
	}

	// Otherwise, return the original size.
	return (chunkInfo[id].compressionType == 0)
				? chunkInfo[id].uncompressedLen
				: chunkInfo[id].len;
}

void DirectorFile::write(Common::WriteStream &stream) {
	stream.endianness = endianness;

	writeChunk(stream, 0); // Write RIFX
	writeChunk(stream, 1); // Write imap
	writeChunk(stream, 2); // Write mmap

	for (auto [id, info] : chunkInfo) {
		if (id <= 2) // Ignore RIFX, imap, mmap
			continue;

		writeChunk(stream, id);
	}
}

void DirectorFile::writeChunk(Common::WriteStream &stream, int32_t id) {
	auto &mapEntry = memoryMap->mapArray[id];
	Chunk *chunk = nullptr;
	switch (id) {
	case 0: // RIFX
		{
			stream.writeUint32(FOURCC('R', 'I', 'F', 'X'));
			stream.writeUint32(size());
			uint32_t newCodec;
			switch (codec) {
			case FOURCC('M', 'C', '9', '5'):
			case FOURCC('F', 'G', 'D', 'C'):
				newCodec = FOURCC('M', 'C', '9', '5');
				break;
			default:
				newCodec = FOURCC('M', 'V', '9', '3');
			}
			stream.writeUint32(newCodec);
		}
		return;
	case 1: // imap
		chunk = initialMap.get();
		break;
	case 2: // mmap
		chunk = memoryMap.get();
		break;
	default:
		if (deserializedChunks.find(id) != deserializedChunks.end()) {
			chunk = deserializedChunks[id].get();
		}
		break;
	}
	stream.seek(mapEntry.offset);
	stream.writeUint32(mapEntry.fourCC);
	stream.writeUint32(mapEntry.len);
	if (chunk && chunk->writable) {
		chunk->write(stream);
		stream.endianness = endianness; // reset endianness
	} else {
		auto chunkData = getChunkData(mapEntry.fourCC, id);
		stream.writeBytes(chunkData->getData(), chunkData->len());
	}
	size_t len = stream.pos() - mapEntry.offset - 8;
	if ((unsigned)mapEntry.len != len) {
		Common::log(
			boost::format("Size estimate for '%s' was incorrect! (Expected %d bytes, wrote %d)")
				% fourCCToString(mapEntry.fourCC) % mapEntry.len % len
		);
	}
}

// restoration

void DirectorFile::restoreScriptText() {
	for (const auto &cast : casts) {
		if (!cast->lctx)
			continue;

		for (auto [scriptId, script] : cast->lctx->scripts) {
			CastMemberChunk *member = script->member;
			if (member) {
				member->info->scriptSrcText = script->scriptText();
			}
		}
	}
}

// dumping

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
