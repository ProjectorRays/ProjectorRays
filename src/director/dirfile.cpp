/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <filesystem>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

#include "common/json.h"
#include "common/log.h"
#include "common/stream.h"
#include "common/util.h"
#include "director/chunk.h"
#include "director/dirfile.h"
#include "director/fontmap.h"
#include "director/guid.h"
#include "director/sound.h"
#include "director/subchunk.h"
#include "director/util.h"
#include "io/fileio.h"

namespace Director {

static const size_t kRIFXHeaderSize = 12;
static const size_t kChunkHeaderSize = 8;

/* DirectorFile */

DirectorFile::DirectorFile() :
	_ilsBodyOffset(0),
	stream(nullptr),
	keyTable(nullptr),
	config(nullptr),
	version(0),
	codec(0),
	afterburned(false) {}

DirectorFile::~DirectorFile() = default;

// read stuff

bool DirectorFile::read(Common::ReadStream *s) {
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
			return false;
	} else {
		Common::warning("Codec unsupported: " + Common::fourCCToString(codec));
		return false;
	}

	if (!readKeyTable())
		return false;
	if (!readConfig())
		return false;
	if (!readCasts())
		return false;

	return true;
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

		Common::debug(boost::format("Found RIFX resource index %d: '%s', %u bytes @ pos 0x%08x (%d)")
						% i % Common::fourCCToString(mapEntry.fourCC) % mapEntry.len % mapEntry.offset % mapEntry.offset);

		ChunkInfo info;
		info.id = i;
		info.fourCC = mapEntry.fourCC;
		info.len = mapEntry.len;
		info.uncompressedLen = mapEntry.len;
		info.offset = mapEntry.offset;
		info.compressionID = NULL_COMPRESSION_GUID;
		chunkInfo[i] = info;

		chunkIDsByFourCC[mapEntry.fourCC].push_back(i);
	}
}

bool DirectorFile::readAfterburnerMap() {
	uint32_t start, end;

	// File version
	if (stream->readUint32() != FOURCC('F', 'v', 'e', 'r')) {
		Common::warning("readAfterburnerMap(): Fver expected but not found");
		return false;
	}

	uint32_t fverLength = stream->readVarInt();
	start = stream->pos();
	uint32_t fverVersion = stream->readVarInt();
	Common::debug(boost::format("Fver: version: 0x%X") % fverVersion);
	if (fverVersion >= 0x401) {
		uint32_t imapVersion = stream->readVarInt();
		uint32_t directorVersion = stream->readVarInt();
		Common::debug(boost::format("Fver: imapVersion: %u directorVersion: 0x%X") % imapVersion % directorVersion);
	}
	if (fverVersion >= 0x501) {
		uint8_t versionStringLen = stream->readUint8();
		fverVersionString = stream->readString(versionStringLen);
		Common::debug(boost::format("Fver: versionString: %s") % fverVersionString);
	}
	end = stream->pos();

	if (end - start != fverLength) {
		Common::warning(boost::format("readAfterburnerMap(): Expected Fver of length %u but read %u bytes")
						% fverLength % (end - start));
		stream->seek(start + fverLength);
	}

	// Compression types
	if (stream->readUint32() != FOURCC('F', 'c', 'd', 'r')) {
		Common::warning("readAfterburnerMap(): Fcdr expected but not found");
		return false;
	}

	uint32_t fcdrLength = stream->readVarInt();
	std::vector<uint8_t> fcdrBuf(10 * fcdrLength); // Should be big enough...
	ssize_t fcdrUncompLength = stream->readZlibBytes(fcdrLength, fcdrBuf.data(), fcdrBuf.size());
	if (fcdrUncompLength == -1) {
		Common::warning("Fcdr: Could not decompress");
		return false;
	}

	Common::ReadStream fcdrStream(fcdrBuf.data(), (unsigned)fcdrUncompLength, endianness);

	uint16_t compressionTypeCount = fcdrStream.readUint16();
	std::vector<MoaID> compressionIDs(compressionTypeCount);
	for (auto &compressionID : compressionIDs) {
		compressionID.read(fcdrStream);
	}
	std::vector<std::string> compressionDescs(compressionTypeCount);
	for (auto &compressionDesc : compressionDescs) {
		compressionDesc = fcdrStream.readCString();
	}
	if (fcdrStream.pos() != (unsigned)fcdrUncompLength) {
		Common::warning(boost::format("readAfterburnerMap(): Fcdr has uncompressed length %zu but read %zu bytes")
						% (unsigned)fcdrUncompLength % fcdrStream.pos());
	}

	Common::debug(boost::format("Fcdr: %u compression types") % compressionTypeCount);
	for (size_t i = 0; i < compressionTypeCount; i++) {
		Common::debug(boost::format("Fcdr: type %zu: %s \"%s\"")
						% i % compressionIDs[i].toString() % compressionDescs[i]);
	}

	// Afterburner map
	if (stream->readUint32() != FOURCC('A', 'B', 'M', 'P')) {
		Common::warning("RIFXArchive::readAfterburnerMap(): ABMP expected but not found");
		return false;
	}
	uint32_t abmpLength = stream->readVarInt();
	uint32_t abmpEnd = stream->pos() + abmpLength;
	uint32_t abmpCompressionType = stream->readVarInt();
	uint32_t abmpUncompLength = stream->readVarInt();
	Common::debug(boost::format("ABMP: length: %u compressionType: %u uncompressedLength: %u")
					% abmpLength % abmpCompressionType % abmpUncompLength);

	std::vector<uint8_t> abmpBuf(abmpUncompLength);
	ssize_t abmpActualUncompLength = stream->readZlibBytes(abmpEnd - stream->pos(), abmpBuf.data(), abmpBuf.size());
	if (abmpActualUncompLength == -1) {
		Common::warning("ABMP: Could not decompress");
		return false;
	}
	if ((unsigned)abmpActualUncompLength != abmpUncompLength) {
		Common::warning(boost::format("ABMP: Expected uncompressed length %u but got length %zu")
						% abmpUncompLength % (unsigned)abmpActualUncompLength);
	}
	Common::ReadStream abmpStream(abmpBuf.data(), abmpBuf.size(), endianness);

	uint32_t abmpUnk1 = abmpStream.readVarInt();
	uint32_t abmpUnk2 = abmpStream.readVarInt();
	uint32_t resCount = abmpStream.readVarInt();
	Common::debug(boost::format("ABMP: unk1: %u unk2: %u resCount: %u")
					% abmpUnk1 % abmpUnk2 % resCount);

	for (uint32_t i = 0; i < resCount; i++) {
		int32_t resId = abmpStream.readVarInt();
		int32_t offset = abmpStream.readVarInt();
		uint32_t compSize = abmpStream.readVarInt();
		uint32_t uncompSize = abmpStream.readVarInt();
		uint32_t compressionType = abmpStream.readVarInt();
		uint32_t tag = abmpStream.readUint32();

		Common::debug(boost::format("Found RIFX resource index %d: '%s', %u bytes (%u uncompressed) @ pos 0x%08x (%d), compressionType: %u")
						% resId % Common::fourCCToString(tag) % compSize % uncompSize % offset % offset % compressionType);

		ChunkInfo info;
		info.id = resId;
		info.fourCC = tag;
		info.len = compSize;
		info.uncompressedLen = uncompSize;
		info.offset = offset;
		info.compressionID = compressionIDs[compressionType];
		chunkInfo[resId] = info;

		chunkIDsByFourCC[tag].push_back(resId);
	}

	// Initial load segment
	if (chunkInfo.find(2) == chunkInfo.end()) {
		Common::warning("readAfterburnerMap(): Map has no entry for ILS");
		return false;
	}
	if (stream->readUint32() != FOURCC('F', 'G', 'E', 'I')) {
		Common::warning("readAfterburnerMap(): FGEI expected but not found");
		return false;
	}

	ChunkInfo &ilsInfo = chunkInfo[2];
	uint32_t ilsUnk1 = stream->readVarInt();
	Common::debug(boost::format("ILS: length: %u unk1: %u") % ilsInfo.len % ilsUnk1);
	_ilsBodyOffset = stream->pos();
	_ilsBuf.resize(ilsInfo.uncompressedLen);
	ssize_t ilsActualUncompLength = stream->readZlibBytes(ilsInfo.len, _ilsBuf.data(), _ilsBuf.size());
	if (ilsActualUncompLength == -1) {
		Common::warning("ILS: Could not decompress");
		return false;
	}
	if ((unsigned)ilsActualUncompLength != ilsInfo.uncompressedLen) {
		Common::warning(boost::format("ILS: Expected uncompressed length %u but got length %zu")
						% ilsInfo.uncompressedLen % (unsigned)ilsActualUncompLength);
	}
	Common::ReadStream ilsStream(_ilsBuf.data(), ilsInfo.uncompressedLen, endianness);

	while (!ilsStream.eof()) {
		int32_t resId = ilsStream.readVarInt();
		ChunkInfo &info = chunkInfo[resId];

		Common::debug(boost::format("Loading ILS resource %d: '%s', %u bytes")
						% resId % Common::fourCCToString(info.fourCC) % info.len);

		_cachedChunkViews[resId] = ilsStream.readByteView(info.len);
	}

	return true;
}

bool DirectorFile::readKeyTable() {
	auto info = getFirstChunkInfo(FOURCC('K', 'E', 'Y', '*'));
	if (info) {
		keyTable = static_cast<KeyTableChunk *>(getChunk(info->fourCC, info->id));

		for (size_t i = 0; i < keyTable->usedCount; i++) {
			const KeyTableEntry &entry = keyTable->entries[i];
			uint32_t ownerTag = FOURCC('?', '?', '?', '?');
			if (chunkInfo.find(entry.castID) != chunkInfo.end()) {
				ownerTag = chunkInfo[entry.castID].fourCC;
			}
			Common::debug(boost::format("KEY* entry %u: '%s' @ %d owned by '%s' @ %d")
				% i % Common::fourCCToString(entry.fourCC) % entry.sectionID % Common::fourCCToString(ownerTag) % entry.castID);
		}

		return true;
	}

	Common::warning("No key chunk!");
	return false;
}

bool DirectorFile::readConfig() {
	auto info = getFirstChunkInfo(FOURCC('D', 'R', 'C', 'F'));
	if (!info)
		info = getFirstChunkInfo(FOURCC('V', 'W', 'C', 'F'));

	if (info) {
		config = static_cast<ConfigChunk *>(getChunk(info->fourCC, info->id));
		version = humanVersion(config->directorVersion);
		dotSyntax = (version >= 700); // TODO: Check for verbose/dot syntax opcodes, allow users to toggle this

		return true;
	}

	Common::warning("No config chunk!");
	return false;
}

bool DirectorFile::readCasts() {
	bool internal = true;

	if (version >= 500) {
		auto info = getFirstChunkInfo(FOURCC('M', 'C', 's', 'L'));
		if (info) {
			CastListChunk *castList = static_cast<CastListChunk *>(getChunk(info->fourCC, info->id));
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
					CastChunk *cast = static_cast<CastChunk *>(getChunk(FOURCC('C', 'A', 'S', '*'), sectionID));
					cast->populate(castEntry.name, castEntry.id, castEntry.minMember);
					casts.push_back(cast);
				}
			}

			return true;
		} else {
			internal = false;
		}
	}

	auto info = getFirstChunkInfo(FOURCC('C', 'A', 'S', '*'));
	if (info) {
		CastChunk *cast = static_cast<CastChunk *>(getChunk(info->fourCC, info->id));
		cast->populate(internal ? "Internal" : "External", 1024, config->minMember);
		casts.push_back(cast);
	}

	return true;
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

Chunk *DirectorFile::getChunk(uint32_t fourCC, int32_t id) {
	if (deserializedChunks.find(id) != deserializedChunks.end())
		return deserializedChunks[id].get();

	Common::BufferView chunkView = getChunkData(fourCC, id);
	deserializedChunks[id] = makeChunk(fourCC, chunkView);
	return deserializedChunks[id].get();
}

Common::BufferView DirectorFile::getChunkData(uint32_t fourCC, int32_t id) {
	if (chunkInfo.find(id) == chunkInfo.end())
		throw std::runtime_error("Could not find chunk " + std::to_string(id));

	auto &info = chunkInfo[id];
	if (fourCC != info.fourCC) {
		throw std::runtime_error(
			"Expected chunk " + std::to_string(id) + " to be '" + Common::fourCCToString(fourCC)
			+ "', but is actually '" + Common::fourCCToString(info.fourCC) + "'"
		);
	}

	if (_cachedChunkViews.find(id) != _cachedChunkViews.end()) {
		return _cachedChunkViews[id];
	}

	if (afterburned) {
		stream->seek(info.offset + _ilsBodyOffset);
		if (info.len == 0 && info.uncompressedLen == 0) {
			_cachedChunkViews[id] = stream->readByteView(info.len);
		} else if (compressionImplemented(info.compressionID)) {
			ssize_t actualUncompLength = -1;
			_cachedChunkBufs[id] = std::vector<uint8_t>(info.uncompressedLen);
			if (info.compressionID == ZLIB_COMPRESSION_GUID) {
				actualUncompLength = stream->readZlibBytes(info.len, _cachedChunkBufs[id].data(), _cachedChunkBufs[id].size());
			} else if (info.compressionID == SND_COMPRESSION_GUID) {
				Common::BufferView chunkView = stream->readByteView(info.len);
				Common::ReadStream chunkStream(chunkView, endianness);
				Common::WriteStream uncompStream(_cachedChunkBufs[id].data(), _cachedChunkBufs[id].size(), endianness);
				actualUncompLength = decompressSnd(chunkStream, uncompStream, id);
			}
			if (actualUncompLength == -1) {
				throw std::runtime_error(boost::str(
					boost::format("Chunk %d: Could not decompress") % id
				));
			}
			if ((unsigned)actualUncompLength != info.uncompressedLen) {
				throw std::runtime_error(boost::str(
					boost::format("Chunk %d: Expected uncompressed length %u but got length %zu")
						% id % info.uncompressedLen % (unsigned)actualUncompLength
				));
			}
			_cachedChunkViews[id] = Common::BufferView(_cachedChunkBufs[id].data(), _cachedChunkBufs[id].size());
		} else if (info.compressionID == FONTMAP_COMPRESSION_GUID) {
			_cachedChunkViews[id] = getFontMap(version);
		} else {
			if (info.compressionID != NULL_COMPRESSION_GUID) {
				Common::warning(boost::format("Unhandled compression type %s!") % info.compressionID.toString());
			}
			_cachedChunkViews[id] = stream->readByteView(info.len);
		}
	} else {
		stream->seek(info.offset);
		_cachedChunkViews[id] = readChunkData(fourCC, info.len);
	}

	return _cachedChunkViews[id];
}

std::shared_ptr<Chunk> DirectorFile::readChunk(uint32_t fourCC, uint32_t len) {
	Common::BufferView chunkView = readChunkData(fourCC, len);
	Common::ReadStream chunkStream(chunkView, endianness);
	return makeChunk(fourCC, chunkStream);
}

Common::BufferView DirectorFile::readChunkData(uint32_t fourCC, uint32_t len) {
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
			+ " expected '" + Common::fourCCToString(fourCC) + "' chunk with length " + std::to_string(len)
			+ ", but got '" + Common::fourCCToString(validFourCC) + "' chunk with length " + std::to_string(validLen)
		);
	} else {
		Common::debug("At offset " + std::to_string(offset) + " reading chunk '" + Common::fourCCToString(fourCC) + "' with length " + std::to_string(len));
	}

	return stream->readByteView(len);
}

std::shared_ptr<Chunk> DirectorFile::makeChunk(uint32_t fourCC, const Common::BufferView &view) {
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
	case FOURCC('L', 'c', 't', 'x'):
	case FOURCC('L', 'c', 't', 'X'):
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
			boost::format("Could not deserialize '%s' chunk") % Common::fourCCToString(fourCC)
		));
		break;
	}

	Common::ReadStream chunkStream(view, endianness);
	res->read(chunkStream);

	return res;
}

LingoDec::Script *DirectorFile::getScript(int32_t id) {
	return static_cast<ScriptChunk *>(getChunk(FOURCC('L', 's', 'c', 'r'), id));
}

LingoDec::ScriptNames *DirectorFile::getScriptNames(int32_t id) {
	return static_cast<ScriptNamesChunk *>(getChunk(FOURCC('L', 'n', 'a', 'm'), id));
}

// compression

bool DirectorFile::compressionImplemented(MoaID compressionID) {
	return compressionID == ZLIB_COMPRESSION_GUID
		|| compressionID == SND_COMPRESSION_GUID;
}

// write stuff

void DirectorFile::writeToFile(const std::filesystem::path &path) {
	generateInitialMap();
	generateMemoryMap();
	std::vector<uint8_t> buf(size());
	Common::WriteStream stream(buf.data(), buf.size(), endianness);
	write(stream);
	IO::writeFile(path, stream);
}

void DirectorFile::generateInitialMap() {
	initialMap = std::make_unique<InitialMapChunk>(this);
	initialMap->version = 1;
	initialMap->mmapOffset = kRIFXHeaderSize + kChunkHeaderSize + initialMap->size();
	initialMap->directorVersion = (version < 500) ? 0 : config->directorVersion;
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
	memoryMap->junkHead2 = -1;

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
	imapEntry.flags = 1;
	imapEntry.unknown0 = 0;
	imapEntry.next = 0;
	nextOffset += imapEntry.len + kChunkHeaderSize;

	auto &mmapEntry = memoryMap->mapArray[2];
	mmapEntry.fourCC = FOURCC('m', 'm', 'a', 'p');
	mmapEntry.len = memoryMap->size();
	mmapEntry.offset = nextOffset;
	mmapEntry.flags = 0;
	mmapEntry.unknown0 = 0;
	mmapEntry.next = 0;
	nextOffset += mmapEntry.len + kChunkHeaderSize;

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
		nextOffset += entry.len + kChunkHeaderSize;
	}

	rifxEntry.len = nextOffset - kChunkHeaderSize;

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
	return memoryMap->mapArray[0].len + kChunkHeaderSize;
}

size_t DirectorFile::chunkSize(int32_t id) {
	// If we've implemented writing for this chunk, recalculate its size.
	if (deserializedChunks.find(id) != deserializedChunks.end()) {
		Chunk &chunk = *deserializedChunks[id];
		if (chunk.writable) {
			return chunk.size();
		}
	}

	auto &info = chunkInfo[id];

	// If this is a compressed fontmap, return the default fontmap size.
	if (info.compressionID == FONTMAP_COMPRESSION_GUID) {
		return getFontMap(version).size();
	}

	// If we've implemented this compression algorithm,
	// return the uncompressed size.
	if (compressionImplemented(info.compressionID)) {
		return info.uncompressedLen;
	}

	// Otherwise, return the original size.
	return info.len;
}

void DirectorFile::write(Common::WriteStream &stream) {
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

	stream.endianness = endianness;

	stream.seek(mapEntry.offset);
	stream.writeUint32(mapEntry.fourCC);
	stream.writeUint32(mapEntry.len);

	Chunk *chunk = nullptr;
	switch (id) {
	case 0: // RIFX
		{
			uint32_t newCodec = (isCast()) ? FOURCC('M', 'C', '9', '5') : FOURCC('M', 'V', '9', '3');
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
	if (chunk && chunk->writable) {
		chunk->write(stream);
	} else {
		stream.writeBytes(getChunkData(mapEntry.fourCC, id));
	}
	size_t len = stream.pos() - mapEntry.offset - 8;
	if ((unsigned)mapEntry.len != len) {
		Common::warning(
			boost::format("Size estimate for '%s' was incorrect! (Expected %u bytes, wrote %zu)")
				% Common::fourCCToString(mapEntry.fourCC) % mapEntry.len % len
		);
	}
}

// restoration

void DirectorFile::parseScripts() {
	for (const auto &cast : casts) {
		if (!cast->lctx)
			continue;

		cast->lctx->parseScripts();
	}
}

void DirectorFile::restoreScriptText() {
	for (const auto &cast : casts) {
		if (!cast->lctx)
			continue;

		for (auto [scriptId, script] : cast->lctx->scripts) {
			CastMemberChunk *member = static_cast<ScriptChunk *>(script)->member;
			if (member) {
				member->setScriptText(script->scriptText("\r", dotSyntax));
			}
		}
	}
}

// dumping

void DirectorFile::dumpScripts(fs::path castsDir) {
	for (const auto &cast : casts) {
		if (!cast->lctx)
			continue;

		fs::path castDir = castsDir / IO::cleanFileName(cast->name);
		fs::create_directory(castDir);

		for (auto it = cast->lctx->scripts.begin(); it != cast->lctx->scripts.end(); ++it) {
			std::string scriptType;
			std::string id;
			CastMemberChunk *member = static_cast<ScriptChunk *>(it->second)->member;
			if (!member)
				continue;

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
			id = std::to_string(member->id);
			if (!member->getName().empty()) {
				id += " - " + member->getName();
			}

			std::string fileName = IO::cleanFileName(scriptType + " " + id);
			IO::writeFile(castDir / (fileName + ".ls"), it->second->scriptText(IO::kPlatformLineEnding, dotSyntax));
			IO::writeFile(castDir / (fileName + ".lasm"), it->second->bytecodeText(IO::kPlatformLineEnding, dotSyntax));
		}
	}
}

void DirectorFile::dumpChunks(fs::path chunksDir) {
	for (auto it = chunkInfo.begin(); it != chunkInfo.end(); it++) {
		const auto &info = it->second;
		if (info.id == 0) // RIFX
			continue;

		std::string fileName = IO::cleanFileName(Common::fourCCToString(info.fourCC) + "-" + std::to_string(info.id)) + ".bin";
		IO::writeFile(chunksDir / fileName, getChunkData(info.fourCC, info.id));
	}
}

void DirectorFile::dumpJSON(fs::path chunksDir) {
	for (auto it = chunkInfo.begin(); it != chunkInfo.end(); it++) {
		const auto &info = it->second;
		if (info.id == 0) // RIFX
			continue;

		std::string fileName = IO::cleanFileName(Common::fourCCToString(info.fourCC) + "-" + std::to_string(info.id)) + ".json";
		if (deserializedChunks.find(info.id) != deserializedChunks.end()) {
			Common::JSONWriter json(IO::kPlatformLineEnding);
			deserializedChunks[info.id]->writeJSON(json);
			IO::writeFile(chunksDir / fileName, json.str());
		}
	}
}

bool DirectorFile::isCast() const {
	return codec == FOURCC('M', 'C', '9', '5') || codec == FOURCC('F', 'G', 'D', 'C');
}

} // namespace Director
