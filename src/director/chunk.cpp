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

#include <boost/format.hpp>

#include "common/log.h"
#include "common/stream.h"
#include "director/castmember.h"
#include "director/chunk.h"
#include "director/lingo.h"
#include "director/dirfile.h"
#include "director/subchunk.h"
#include "director/util.h"

namespace Director {

/* Chunk */

void to_json(ordered_json &j, const Chunk &c) {
	switch (c.chunkType) {
	case kCastChunk:
		to_json(j, static_cast<const CastChunk &>(c));
		break;
	case kCastListChunk:
		to_json(j, static_cast<const CastListChunk &>(c));
		break;
	case kCastMemberChunk:
		to_json(j, static_cast<const CastMemberChunk &>(c));
		break;
	case kCastInfoChunk:
		to_json(j, static_cast<const CastInfoChunk &>(c));
		break;
	case kConfigChunk:
		to_json(j, static_cast<const ConfigChunk &>(c));
		break;
	case kInitialMapChunk:
		to_json(j, static_cast<const InitialMapChunk &>(c));
		break;
	case kKeyTableChunk:
		to_json(j, static_cast<const KeyTableChunk &>(c));
		break;
	case kMemoryMapChunk:
		to_json(j, static_cast<const MemoryMapChunk &>(c));
		break;
	case kScriptChunk:
		to_json(j, static_cast<const ScriptChunk &>(c));
		break;
	case kScriptContextChunk:
		to_json(j, static_cast<const ScriptContextChunk &>(c));
		break;
	case kScriptNamesChunk:
		to_json(j, static_cast<const ScriptNamesChunk &>(c));
		break;
	}
}

/* CastChunk */

void CastChunk::read(Common::ReadStream &stream) {
	stream.endianness = Common::kBigEndian;
	while (!stream.eof()) {
		auto id = stream.readInt32();
		memberIDs.push_back(id);
	}
}

void to_json(ordered_json &j, const CastChunk &c) {
	j["memberIDs"] = c.memberIDs;
}

void CastChunk::populate(const std::string &castName, int32_t id, uint16_t minMember) {
	name = castName;

	for (const auto &entry : dir->keyTable->entries) {
		if (entry.castID == id
				&& (entry.fourCC == FOURCC('L', 'c', 't', 'x') || entry.fourCC == FOURCC('L', 'c', 't', 'X'))
				&& dir->chunkExists(entry.fourCC, entry.sectionID)) {
			lctx = std::static_pointer_cast<ScriptContextChunk>(dir->getChunk(entry.fourCC, entry.sectionID));
			break;
		}
	}

	for (size_t i = 0; i < memberIDs.size(); i++) {
		int32_t sectionID = memberIDs[i];
		if (sectionID > 0) {
			auto member = std::static_pointer_cast<CastMemberChunk>(dir->getChunk(FOURCC('C', 'A', 'S', 't'), sectionID));
			member->id = i + minMember;
			Common::debug(boost::format("Member %u: name: \"%s\" chunk: %d")
							% member->id % member->getName() % sectionID);
			if (!member->info) {
				Common::debug(boost::format("Member %u: No info!") % member->id);
			}
			if (lctx && (lctx->scripts.find(member->getScriptID()) != lctx->scripts.end())) {
				member->script = lctx->scripts[member->getScriptID()].get();
				member->script->member = member.get();
			}
			members[member->id] = std::move(member);
		}
	}
}

/* CastListChunk */

void CastListChunk::read(Common::ReadStream &stream) {
	stream.endianness = Common::kBigEndian;
	ListChunk::read(stream);
	entries.resize(castCount);
	for (int i = 0; i < castCount; i++) {
		if (itemsPerCast >= 1)
			entries[i].name = readPascalString(i * itemsPerCast + 1);
		if (itemsPerCast >= 2)
			entries[i].filePath = readPascalString(i * itemsPerCast + 2);
		if (itemsPerCast >= 3)
			entries[i].preloadSettings = readUint16(i * itemsPerCast + 3);
		if (itemsPerCast >= 4) {
			Common::ReadStream item(items[i * itemsPerCast + 4], itemEndianness);
			entries[i].minMember = item.readUint16();
			entries[i].maxMember = item.readUint16();
			entries[i].id = item.readInt32();
		}
	}
}

void CastListChunk::readHeader(Common::ReadStream &stream) {
	dataOffset = stream.readUint32();
	unk0 = stream.readUint16();
	castCount = stream.readUint16();
	itemsPerCast = stream.readUint16();
	unk1 = stream.readUint16();
}

void to_json(ordered_json &j, const CastListChunk &c) {
	j["dataOffset"] = c.dataOffset;
	j["unk0"] = c.unk0;
	j["castCount"] = c.itemsPerCast;
	j["itemsPerCast"] = c.itemsPerCast;
	j["unk1"] = c.unk1;
	j["entries"] = c.entries;
}

/* CastMemberChunk */

void CastMemberChunk::read(Common::ReadStream &stream) {
	stream.endianness = Common::kBigEndian;

	if (dir->version >= 500) {
		type = static_cast<MemberType>(stream.readUint32());
		infoLen = stream.readUint32();
		specificDataLen = stream.readUint32();

		// info
		if (infoLen) {
			Common::ReadStream infoStream(stream.readByteView(infoLen), stream.endianness);
			info = std::make_shared<CastInfoChunk>(dir);
			info->read(infoStream);
		}

		// specific data
		hasFlags1 = false;
		specificData = stream.readByteView(specificDataLen);
	} else {
		specificDataLen = stream.readUint16();
		infoLen = stream.readUint32();

		// these bytes are common but stored in the specific data
		uint32_t specificDataLeft = specificDataLen;
		type = static_cast<MemberType>(stream.readUint8());
		specificDataLeft -= 1;
		if (specificDataLeft) {
			hasFlags1 = true;
			flags1 = stream.readUint8();
			specificDataLeft -= 1;
		} else {
			hasFlags1 = false;
		}

		// specific data
		specificData = stream.readByteView(specificDataLeft);

		// info
		Common::ReadStream infoStream (stream.readByteView(infoLen), stream.endianness);
		if (infoLen) {
			info = std::make_shared<CastInfoChunk>(dir);
			info->read(infoStream);
		}
	}

	switch (type) {
	case kScriptMember:
		member = std::make_unique<ScriptMember>(dir);
		break;
	default:
		member = std::make_unique<CastMember>(dir, type);
		break;
	}
	Common::ReadStream specificStream(specificData, stream.endianness);
	member->read(specificStream);
}

size_t CastMemberChunk::size() {
	infoLen = info ? info->size() : 0;
	specificDataLen = specificData.size();

	size_t len = 0;
	if (dir->version >= 500) {
		len += 4; // type
		len += 4; // infoLen
		len += 4; // specificDataLen
		len += infoLen; // info
		len += specificDataLen; // specificData
	} else {
		specificDataLen += 1; // type
		if (hasFlags1) {
			specificDataLen += 1; // flags1
		}

		len += 2; // specificDataLen
		len += 4; // infoLen
		len += specificDataLen; // specificData
		len += infoLen; // info
	}
	return len;
}

void CastMemberChunk::write(Common::WriteStream &stream) {
	stream.endianness = Common::kBigEndian;

	if (dir->version >= 500) {
		stream.writeUint32(type);
		stream.writeUint32(infoLen);
		stream.writeUint32(specificDataLen);
		if (info) {
			info->write(stream);
		}
		stream.writeBytes(specificData);
	} else {
		stream.writeUint16(specificDataLen);
		stream.writeUint32(infoLen);
		stream.writeUint8(type);
		if (hasFlags1) {
			stream.writeUint8(flags1);
		}
		stream.writeBytes(specificData);
		if (info) {
			info->write(stream);
		}
	}
}

uint32_t CastMemberChunk::getScriptID() const {
	if (info) {
		return info->scriptId;
	}
	return 0;
}

std::string CastMemberChunk::getScriptText() const {
	if (info) {
		return info->scriptSrcText;
	}
	return "";
}

void CastMemberChunk::setScriptText(std::string val) {
	if (!info) {
		Common::warning("Tried to set scriptText on member with no info!");
		return;
	}
	info->scriptSrcText = val;
}

std::string CastMemberChunk::getName() const {
	if (info) {
		return info->name;
	}
	return "";
}

void to_json(ordered_json &j, const CastMemberChunk &c) {
	j["type"] = c.type;
	j["infoLen"] = c.infoLen;
	if (c.hasFlags1) {
		j["flags1"] = c.flags1;
	}
	j["specificDataLen"] = c.specificDataLen;
	j["info"] = *c.info;
	j["member"] = *c.member;
}

/* CastInfoChunk */

void CastInfoChunk::read(Common::ReadStream &stream) {
	ListChunk::read(stream);
	scriptSrcText = readString(0);
	name = readPascalString(1);
	// cProp02 = readProperty(2);
	// cProp03 = readProperty(3);
	// comment = readString(4);
	// cProp05 = readProperty(5);
	// cProp06 = readProperty(6);
	// cProp07 = readProperty(7);
	// cProp08 = readProperty(8);
	// xtraGUID = readProperty(9);
	// cProp10 = readProperty(10);
	// cProp11 = readProperty(11);
	// cProp12 = readProperty(12);
	// cProp13 = readProperty(13);
	// cProp14 = readProperty(14);
	// cProp15 = readProperty(15);
	// fileFormatID = readString(16);
	// created = readUint32(17);
	// modified = readUint32(18);
	// cProp19 = readProperty(19);
	// cProp20 = readProperty(20);
	// imageCompression = readProperty(21);
}

void CastInfoChunk::readHeader(Common::ReadStream &stream) {
	dataOffset = stream.readUint32();
	unk1 = stream.readUint32();
	unk2 = stream.readUint32();
	flags = stream.readUint32();
	scriptId = stream.readUint32();
}

size_t CastInfoChunk::headerSize() {
	size_t len = 0;
	len += 4; // dataOffset
	len += 4; // unk1
	len += 4; // unk2
	len += 4; // flags
	len += 4; // scriptId
	return len;
}

void CastInfoChunk::writeHeader(Common::WriteStream &stream) {
	stream.writeUint32(headerSize());
	stream.writeUint32(unk1);
	stream.writeUint32(unk2);
	stream.writeUint32(flags);
	stream.writeUint32(scriptId);
}

size_t CastInfoChunk::itemSize(uint16_t index) {
	switch (index) {
	case 0:
		return scriptSrcText.size();
	case 1:
		return (name.size() > 0) ? 1 + name.size() : 0;
	default:
		return ListChunk::itemSize(index);
	}
}

void CastInfoChunk::writeItem(Common::WriteStream &stream, uint16_t index) {
	switch (index) {
	case 0:
		stream.writeString(scriptSrcText);
		break;
	case 1:
		if (name.size() > 0) {
			stream.writePascalString(name);
		}
		break;
	default:
		ListChunk::writeItem(stream, index);
		break;
	}
}

void to_json(ordered_json &j, const CastInfoChunk &c) {
	j["dataOffset"] = c.dataOffset;
	j["unk1"] = c.unk1;
	j["unk2"] = c.unk2;
	j["flags"] = c.flags;
	j["scriptId"] = c.scriptId;
	j["scriptSrcText"] = c.scriptSrcText;
	j["name"] = c.name;
	// j["comment"] = c.comment;
	// j["fileFormatID"] = c.fileFormatID;
	// j["created"] = c.created;
	// j["modified"] = c.modified;
}

/* ConfigChunk */

void ConfigChunk::read(Common::ReadStream &stream) {
	stream.endianness = Common::kBigEndian;

	/*  0 */ len = stream.readInt16();
	/*  2 */ fileVersion = stream.readInt16();
	/*  4 */ movieTop = stream.readInt16();
	/*  6 */ movieLeft = stream.readInt16();
	/*  8 */ movieBottom = stream.readInt16();
	/* 10 */ movieRight = stream.readInt16();
	/* 12 */ minMember = stream.readInt16();
	/* 14 */ maxMember = stream.readInt16();
	/* 16 */ field9 = stream.readInt8();
	/* 17 */ field10 = stream.readInt8();
	/* 18 */ field11 = stream.readInt16();
	/* 20 */ commentFont = stream.readInt16();
	/* 22 */ commentSize = stream.readInt16();
	/* 24 */ commentStyle = stream.readUint16();
	/* 26 */ stageColor = stream.readInt16();
	/* 28 */ bitDepth = stream.readInt16();
	/* 30 */ field17 = stream.readUint8();
	/* 31 */ field18 = stream.readUint8();
	/* 32 */ field19 = stream.readInt32();
	/* 36 */ directorVersion = stream.readInt16();
	/* 38 */ field21 = stream.readInt16();
	/* 40 */ field22 = stream.readInt32();
	/* 44 */ field23 = stream.readInt32();
	/* 48 */ field24 = stream.readInt32();
	/* 52 */ field25 = stream.readInt8();
	/* 53 */ field26 = stream.readUint8();
	/* 54 */ frameRate = stream.readInt16();
	/* 56 */ platform = stream.readInt16();
	/* 58 */ protection = stream.readInt16();
	/* 60 */ field29 = stream.readInt32();
	/* 64 */ checksum = stream.readUint32();
	/* 68 */ remnants = stream.readByteView(len - stream.pos());

	uint32_t computedChecksum = computeChecksum();
	if (checksum != computedChecksum) {
		Common::warning(boost::format("Checksums don't match! Stored: %u Computed: %u") % checksum % computedChecksum);
		writable = false;
	}
}

size_t ConfigChunk::size() {
	return len;
}

void ConfigChunk::write(Common::WriteStream &stream) {
	stream.endianness = Common::kBigEndian;

	checksum = computeChecksum();

	/*  0 */ stream.writeInt16(len);
	/*  2 */ stream.writeInt16(fileVersion);
	/*  4 */ stream.writeInt16(movieTop);
	/*  6 */ stream.writeInt16(movieLeft);
	/*  8 */ stream.writeInt16(movieBottom);
	/* 10 */ stream.writeInt16(movieRight);
	/* 12 */ stream.writeInt16(minMember);
	/* 14 */ stream.writeInt16(maxMember);
	/* 16 */ stream.writeInt8(field9);
	/* 17 */ stream.writeInt8(field10);
	/* 18 */ stream.writeInt16(field11);
	/* 20 */ stream.writeInt16(commentFont);
	/* 22 */ stream.writeInt16(commentSize);
	/* 24 */ stream.writeUint16(commentStyle);
	/* 26 */ stream.writeInt16(stageColor);
	/* 28 */ stream.writeInt16(bitDepth);
	/* 30 */ stream.writeUint8(field17);
	/* 31 */ stream.writeUint8(field18);
	/* 32 */ stream.writeInt32(field19);
	/* 36 */ stream.writeInt16(directorVersion);
	/* 38 */ stream.writeInt16(field21);
	/* 40 */ stream.writeInt32(field22);
	/* 44 */ stream.writeInt32(field23);
	/* 48 */ stream.writeInt32(field24);
	/* 52 */ stream.writeInt8(field25);
	/* 53 */ stream.writeUint8(field26);
	/* 54 */ stream.writeInt16(frameRate);
	/* 56 */ stream.writeInt16(platform);
	/* 58 */ stream.writeInt16(protection);
	/* 60 */ stream.writeInt32(field29);
	/* 64 */ stream.writeUint32(checksum);
	/* 68 */ stream.writeBytes(remnants);
}

uint32_t ConfigChunk::computeChecksum() {
	unsigned int ver = humanVersion(directorVersion);

	uint32_t check = len + 1;
	Common::debug(boost::format("Checksum step 1 (= %1% + 1): %2%") % len % check);

	check *= fileVersion + 2;
	Common::debug(boost::format("Checksum step 2 (*= %1% + 2): %2%") % fileVersion % check);

	check /= movieTop + 3;
	Common::debug(boost::format("Checksum step 3 (/= %1% + 3): %2%") % movieTop % check);

	check *= movieLeft + 4;
	Common::debug(boost::format("Checksum step 4 (*= %1% + 4): %2%") % movieLeft % check);

	check /= movieBottom + 5;
	Common::debug(boost::format("Checksum step 5 (/= %1% + 5): %2%") % movieBottom % check);

	check *= movieRight + 6;
	Common::debug(boost::format("Checksum step 6 (*= %1% + 6): %2%") % movieRight % check);

	check -= minMember + 7;
	Common::debug(boost::format("Checksum step 7 (-= %1% + 7): %2%") % minMember % check);

	check *= maxMember + 8;
	Common::debug(boost::format("Checksum step 8 (*= %1% + 8): %2%") % maxMember % check);

	check -= field9 + 9;
	Common::debug(boost::format("Checksum step 9 (-= %1% + 9): %2%") % (int)field9 % check);

	check -= field10 + 10;
	Common::debug(boost::format("Checksum step 10 (-= %1% + 10): %2%") % (int)field10 % check);

	check += field11 + 11;
	Common::debug(boost::format("Checksum step 11 (+= %1% + 11): %2%") % field11 % check);

	check *= commentFont + 12;
	Common::debug(boost::format("Checksum step 12 (*= %1% + 12): %2%") % commentFont % check);

	check += commentSize + 13;
	Common::debug(boost::format("Checksum step 13 (+= %1% + 13): %2%") % commentSize % check);

	int32_t operand14 = (ver < 800) ? (uint8_t)((commentStyle >> 8) & 0xFF) : commentStyle;
	check *= operand14 + 14;
	Common::debug(boost::format("Checksum step 14 (*= %1% + 14): %2%") % operand14 % check);

	int32_t operand15 = (ver < 700) ? stageColor : (uint8_t)(stageColor & 0xFF);
	check += operand15 + 15;
	Common::debug(boost::format("Checksum step 15 (+= %1% + 15): %2%") % operand15 % check);

	check += bitDepth + 16;
	Common::debug(boost::format("Checksum step 16 (+= %1% + 16): %2%") % bitDepth % check);

	check += field17 + 17;
	Common::debug(boost::format("Checksum step 17 (+= %1% + 17): %2%") % (unsigned int)field17 % check);

	check *= field18 + 18;
	Common::debug(boost::format("Checksum step 18 (*= %1% + 18): %2%") % (unsigned int)field18 % check);

	check += field19 + 19;
	Common::debug(boost::format("Checksum step 19 (+= %1% + 19): %2%") % field19 % check);

	check *= directorVersion + 20;
	Common::debug(boost::format("Checksum step 20 (*= %1% + 20): %2%") % directorVersion % check);

	check += field21 + 21;
	Common::debug(boost::format("Checksum step 21 (+= %1% + 21): %2%") % field21 % check);

	check += field22 + 22;
	Common::debug(boost::format("Checksum step 22 (+= %1% + 22): %2%") % field22 % check);

	check += field23 + 23;
	Common::debug(boost::format("Checksum step 23 (+= %1% + 23): %2%") % field23 % check);

	check += field24 + 24;
	Common::debug(boost::format("Checksum step 24 (+= %1% + 24): %2%") % field24 % check);

	check *= field25 + 25;
	Common::debug(boost::format("Checksum step 25 (*= %1% + 25): %2%") % (int)field25 % check);

	check += frameRate + 26;
	Common::debug(boost::format("Checksum step 26 (+= %1% + 26): %2%") % frameRate % check);

	check *= platform + 27;
	Common::debug(boost::format("Checksum step 27 (*= %1% + 27): %2%") % platform % check);

	check *= (protection * 0xE06) + 0xFF450000;
	Common::debug(boost::format("Checksum step 28 (*= (%1% * 0xE06) + 0xFF450000): %2%") % protection % check);

	check ^= FOURCC('r', 'a', 'l', 'f');
	Common::debug(boost::format("Checksum step 29 (^= ralf): %1%") % check);

	return check;
}

void ConfigChunk::unprotect() {
	fileVersion = directorVersion;
	if (protection % 23 == 0) {
		protection += 1;
	}
}

void to_json(ordered_json &j, const ConfigChunk &c) {
	j["len"] = c.len;
	j["fileVersion"] = c.fileVersion;
	j["movieTop"] = c.movieTop;
	j["movieLeft"] = c.movieLeft;
	j["movieBottom"] = c.movieBottom;
	j["movieRight"] = c.movieRight;
	j["minMember"] = c.minMember;
	j["maxMember"] = c.maxMember;
	j["field9"] = c.field9;
	j["field10"] = c.field10;
	j["field11"] = c.field11;
	j["commentFont"] = c.commentFont;
	j["commentSize"] = c.commentSize;
	j["commentStyle"] = c.commentStyle;
	j["stageColor"] = c.stageColor;
	j["bitDepth"] = c.bitDepth;
	j["field17"] = c.field17;
	j["field18"] = c.field18;
	j["field19"] = c.field19;
	j["directorVersion"] = c.directorVersion;
	j["field21"] = c.field21;
	j["field22"] = c.field22;
	j["field23"] = c.field23;
	j["field24"] = c.field24;
	j["field25"] = c.field25;
	j["field26"] = c.field26;
	j["frameRate"] = c.frameRate;
	j["platform"] = c.platform;
	j["protection"] = c.protection;
	j["field29"] = c.field29;
	j["checksum"] = c.checksum;
}

/* InitialMapChunk */

void InitialMapChunk::read(Common::ReadStream &stream) {
	one = stream.readUint32();
	mmapOffset = stream.readUint32();
	version = stream.readUint32();
	unused1 = stream.readUint32();
	unused2 = stream.readUint32();
	unused3 = stream.readUint32();
}

size_t InitialMapChunk::size() {
	return 24;
}

void InitialMapChunk::write(Common::WriteStream &stream) {
	stream.writeUint32(one);
	stream.writeUint32(mmapOffset);
	stream.writeUint32(version);
	stream.writeUint32(unused1);
	stream.writeUint32(unused2);
	stream.writeUint32(unused3);
}

void to_json(ordered_json &j, const InitialMapChunk &c) {
	j["one"] = c.one;
	j["mmapOffset"] = c.mmapOffset;
	j["version"] = c.version;
	j["unused1"] = c.unused1;
	j["unused2"] = c.unused2;
	j["unused3"] = c.unused3;
}

/* KeyTableChunk */

void KeyTableChunk::read(Common::ReadStream &stream) {
	entrySize = stream.readUint16();
	entrySize2 = stream.readUint16();
	entryCount = stream.readUint32();
	usedCount = stream.readUint32();

	entries.resize(entryCount);
	for (auto &entry : entries) {
		entry.read(stream);
	}
}

void to_json(ordered_json &j, const KeyTableChunk &c) {
	j["entrySize"] = c.entrySize;
	j["entrySize2"] = c.entrySize2;
	j["entryCount"] = c.entryCount;
	j["usedCount"] = c.usedCount;
	j["entries"] = c.entries;
}

/* ListChunk */

// read stuff

void ListChunk::read(Common::ReadStream &stream) {
	readHeader(stream);
	readOffsetTable(stream);
	readItems(stream);
}

void ListChunk::readHeader(Common::ReadStream &stream) {
	dataOffset = stream.readUint32();
}

void ListChunk::readOffsetTable(Common::ReadStream &stream) {
	stream.seek(dataOffset);
	offsetTableLen = stream.readUint16();
	offsetTable.resize(offsetTableLen);
	for (uint16_t i = 0; i < offsetTableLen; i++) {
		offsetTable[i] = stream.readUint32();
	}
}

void ListChunk::readItems(Common::ReadStream &stream) {
	itemsLen = stream.readUint32();

	itemEndianness = stream.endianness;
	size_t listOffset = stream.pos();

	items.resize(offsetTableLen);
	for (uint16_t i = 0; i < offsetTableLen; i++) {
		size_t offset = offsetTable[i];
		size_t nextOffset = (i == offsetTableLen - 1) ? itemsLen : offsetTable[i + 1];
		stream.seek(listOffset + offset);
		items[i] = stream.readByteView(nextOffset - offset);
	}
}

std::string ListChunk::readString(uint16_t index) {
	if (index >= offsetTableLen)
		return "";

	Common::ReadStream stream(items[index], itemEndianness);
	return stream.readString(stream.size());
}

std::string ListChunk::readPascalString(uint16_t index) {
	if (index >= offsetTableLen)
		return "";

	Common::ReadStream stream(items[index], itemEndianness);
	if (stream.size() == 0)
		return "";

	return stream.readPascalString();
}

uint16_t ListChunk::readUint16(uint16_t index) {
	if (index >= offsetTableLen)
		return 0;

	Common::ReadStream stream(items[index], itemEndianness);
	return stream.readUint16();
}

uint32_t ListChunk::readUint32(uint16_t index) {
	if (index >= offsetTableLen)
		return 0;

	Common::ReadStream stream(items[index], itemEndianness);
	return stream.readUint32();
}

// offset updating

void ListChunk::updateOffsets() {
	uint32_t offset = 0;
	for (uint16_t i = 0; i < offsetTableLen; i++) {
		offsetTable[i] = offset;
		offset += itemSize(i);
	}
	itemsLen = offset;
}

// size stuff

size_t ListChunk::size() {
	size_t len = 0;
	len += headerSize();
	len += offsetTableSize();
	len += itemsSize();
	return len;
}

size_t ListChunk::headerSize() {
	return 4; // dataOffset
}

size_t ListChunk::offsetTableSize() {
	size_t len = 0;
	len += 2; // offsetTableLen;
	len += 4 * offsetTableLen; // offset table
	return len;
}

size_t ListChunk::itemsSize() {
	updateOffsets();
	size_t len = 0;
	len += 4; // itemsLen
	len += itemsLen; // items
	return len;
}

size_t ListChunk::itemSize(uint16_t index) {
	return items[index].size();
}

// write stuff

void ListChunk::write(Common::WriteStream &stream) {
	writeHeader(stream);
	writeOffsetTable(stream);
	writeItems(stream);
}

void ListChunk::writeHeader(Common::WriteStream &stream) {
	stream.writeUint32(headerSize());
}

void ListChunk::writeOffsetTable(Common::WriteStream &stream) {
	updateOffsets();
	stream.writeUint16(offsetTableLen);
	for (uint16_t i = 0; i < offsetTableLen; i++) {
		stream.writeUint32(offsetTable[i]);
	}
}

void ListChunk::writeItems(Common::WriteStream &stream) {
	stream.writeUint32(itemsLen);
	for (uint16_t i = 0; i < offsetTableLen; i++) {
		writeItem(stream, i);
	}
}

void ListChunk::writeItem(Common::WriteStream &stream, uint16_t index) {
	stream.writeBytes(items[index]);
}

/* MemoryMapChunk */

void MemoryMapChunk::read(Common::ReadStream &stream) {
	headerLength = stream.readUint16();
	entryLength = stream.readUint16();
	chunkCountMax = stream.readInt32();
	chunkCountUsed = stream.readInt32();
	junkHead = stream.readInt32();
	junkHead2 = stream.readInt32();
	freeHead = stream.readInt32();
	mapArray.resize(chunkCountUsed);
	for (auto &entry : mapArray) {
		entry.read(stream);
	}
}

size_t MemoryMapChunk::size() {
	return headerLength + chunkCountMax * entryLength;
}

void MemoryMapChunk::write(Common::WriteStream &stream) {
	stream.writeUint16(headerLength);
	stream.writeUint16(entryLength);
	stream.writeInt32(chunkCountMax);
	stream.writeInt32(chunkCountUsed);
	stream.writeInt32(junkHead);
	stream.writeInt32(junkHead2);
	stream.writeInt32(freeHead);
	for (auto &entry : mapArray) {
		entry.write(stream);
	}
}

void to_json(ordered_json &j, const MemoryMapChunk &c) {
	j["headerLength"] = c.headerLength;
	j["entryLength"] = c.entryLength;
	j["chunkCountMax"] = c.chunkCountMax;
	j["chunkCountUsed"] = c.chunkCountUsed;
	j["junkHead"] = c.junkHead;
	j["junkHead2"] = c.junkHead2;
	j["freeHead"] = c.freeHead;
	j["mapArray"] = c.mapArray;
}

/* ScriptChunk */

ScriptChunk::ScriptChunk(DirectorFile *m) :
	Chunk(m, kScriptChunk),
	context(nullptr),
	member(nullptr) {}

ScriptChunk::~ScriptChunk() = default;

void ScriptChunk::read(Common::ReadStream &stream) {
	stream.seek(8);
	// Lingo scripts are always big endian regardless of file endianness
	stream.endianness = Common::kBigEndian;
	totalLength = stream.readUint32();
	totalLength2 = stream.readUint32();
	headerLength = stream.readUint16();
	scriptNumber = stream.readUint16();
	stream.seek(38);
	scriptBehavior = stream.readUint32();
	stream.seek(50);
	handlerVectorsCount = stream.readUint16();
	handlerVectorsOffset = stream.readUint32();
	handlerVectorsSize = stream.readUint32();
	propertiesCount = stream.readUint16();
	propertiesOffset = stream.readUint32();
	globalsCount = stream.readUint16();
	globalsOffset = stream.readUint32();
	handlersCount = stream.readUint16();
	handlersOffset = stream.readUint32();
	literalsCount = stream.readUint16();
	literalsOffset = stream.readUint32();
	literalsDataCount = stream.readUint32();
	literalsDataOffset = stream.readUint32();
	propertyNameIDs = readVarnamesTable(stream, propertiesCount, propertiesOffset);
	globalNameIDs = readVarnamesTable(stream, globalsCount, globalsOffset);

	stream.seek(handlersOffset);
	handlers.resize(handlersCount);
	for (auto &handler : handlers) {
		handler = std::make_unique<Handler>(this);
		handler->readRecord(stream);
	}
	for (const auto &handler : handlers) {
		handler->readData(stream);
	}

	stream.seek(literalsOffset);
	literals.resize(literalsCount);
	for (auto &literal : literals) {
		literal.readRecord(stream, dir->version);
	}
	for (auto &literal : literals) {
		literal.readData(stream, literalsDataOffset);
	}
}

std::vector<int16_t> ScriptChunk::readVarnamesTable(Common::ReadStream &stream, uint16_t count, uint32_t offset) {
	stream.seek(offset);
	std::vector<int16_t> nameIDs(count);
	for (uint16_t i = 0; i < count; i++) {
		nameIDs[i] = stream.readInt16();
	}
	return nameIDs;
}

void to_json(ordered_json &j, const ScriptChunk &c) {
	j["totalLength"] = c.totalLength;
	j["totalLength2"] = c.totalLength2;
	j["headerLength"] = c.headerLength;
	j["scriptNumber"] = c.scriptNumber;
	j["scriptBehavior"] = c.scriptBehavior;
	j["handlerVectorsCount"] = c.handlerVectorsCount;
	j["handlerVectorsOffset"] = c.handlerVectorsOffset;
	j["handlerVectorsSize"] = c.handlerVectorsSize;
	j["propertiesCount"] = c.propertiesCount;
	j["propertiesOffset"] = c.propertiesOffset;
	j["globalsCount"] = c.globalsCount;
	j["globalsOffset"] = c.globalsOffset;
	j["handlersCount"] = c.handlersCount;
	j["handlersOffset"] = c.handlersOffset;
	j["literalsCount"] = c.literalsCount;
	j["literalsOffset"] = c.literalsOffset;
	j["literalsDataCount"] = c.literalsDataCount;
	j["literalsDataOffset"] = c.literalsDataOffset;
	j["propertyNameIDs"] = c.propertyNameIDs;
	j["globalNameIDs"] = c.globalNameIDs;
	ordered_json handlers = ordered_json::array();
	for (const auto &handler : c.handlers)
		handlers.push_back(*handler);
	j["handlers"] = handlers;
	j["literals"] = c.literals;
}

std::string ScriptChunk::getName(int id) {
	return context->getName(id);
}

void ScriptChunk::setContext(ScriptContextChunk *ctx) {
	this->context = ctx;
	for (auto nameID : propertyNameIDs) {
		propertyNames.push_back(getName(nameID));
	}
	for (auto nameID : globalNameIDs) {
		globalNames.push_back(getName(nameID));
	}
	for (const auto &handler : handlers) {
		handler->readNames();
	}
}

void ScriptChunk::translate() {
	for (const auto &handler : handlers) {
		handler->translate();
	}
}

std::string ScriptChunk::varDeclarations() {
	std::string res = "";
	if (propertyNames.size() > 0) {
		res += "property ";
		for (size_t i = 0; i < propertyNames.size(); i++) {
			if (i > 0)
				res += ", ";
			res += propertyNames[i];
		}
		res += kLingoLineEnding;
	}
	if (globalNames.size() > 0) {
		res += "global ";
		for (size_t i = 0; i < globalNames.size(); i++) {
			if (i > 0)
				res += ", ";
			res += globalNames[i];
		}
		res += kLingoLineEnding;
	}
	return res;
}

std::string ScriptChunk::scriptText() {
	std::string res = varDeclarations();
	for (size_t i = 0; i < handlers.size(); i++) {
		if (res.size() > 0)
			res += kLingoLineEnding;
		res += handlers[i]->ast->toString(dir->dotSyntax, false);
	}
	return res;
}

std::string ScriptChunk::bytecodeText() {
	std::string res = varDeclarations();
	for (size_t i = 0; i < handlers.size(); i++) {
		if (res.size() > 0)
			res += kLingoLineEnding;
		res += handlers[i]->bytecodeText();
	}
	return res;
}

/* ScriptContextChunk */

void ScriptContextChunk::read(Common::ReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	stream.endianness = Common::kBigEndian;

	unknown0 = stream.readInt32();
	unknown1 = stream.readInt32();
	entryCount = stream.readUint32();
	entryCount2 = stream.readUint32();
	entriesOffset = stream.readUint16();
	unknown2 = stream.readInt16();
	unknown3 = stream.readInt32();
	unknown4 = stream.readInt32();
	unknown5 = stream.readInt32();
	lnamSectionID = stream.readInt32();
	validCount = stream.readUint16();
	flags = stream.readUint16();
	freePointer = stream.readInt16();

	stream.seek(entriesOffset);
	sectionMap.resize(entryCount);
	for (auto &entry : sectionMap) {
		entry.read(stream);
	}

	lnam = std::static_pointer_cast<ScriptNamesChunk>(dir->getChunk(FOURCC('L', 'n', 'a', 'm'), lnamSectionID));
	for (uint32_t i = 1; i <= entryCount; i++) {
		auto section = sectionMap[i - 1];
		if (section.sectionID > -1) {
			auto script = std::static_pointer_cast<ScriptChunk>(dir->getChunk(FOURCC('L', 's', 'c', 'r'), section.sectionID));
			script->setContext(this);
			scripts[i] = script;
		}
	}

	for (auto it = scripts.begin(); it != scripts.end(); ++it) {
		it->second->translate();
	}
}

void to_json(ordered_json &j, const ScriptContextChunk &c) {
	j["unknown0"] = c.unknown0;
	j["unknown1"] = c.unknown1;
	j["entryCount"] = c.entryCount;
	j["entryCount2"] = c.entryCount2;
	j["entriesOffset"] = c.entriesOffset;
	j["unknown2"] = c.unknown2;
	j["unknown3"] = c.unknown3;
	j["unknown4"] = c.unknown4;
	j["unknown5"] = c.unknown5;
	j["lnamSectionID"] = c.lnamSectionID;
	j["validCount"] = c.validCount;
	j["flags"] = c.flags;
	j["freePointer"] = c.freePointer;
	j["sectionMap"] = c.sectionMap;
}

std::string ScriptContextChunk::getName(int id) {
	return lnam->getName(id);
}

/* ScriptNamesChunk */

void ScriptNamesChunk::read(Common::ReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	stream.endianness = Common::kBigEndian;

	unknown0 = stream.readInt32();
	unknown1 = stream.readInt32();
	len1 = stream.readUint32();
	len2 = stream.readUint32();
	namesOffset = stream.readUint16();
	namesCount = stream.readUint16();

	stream.seek(namesOffset);
	names.resize(namesCount);
	for (auto &name : names) {
		auto length = stream.readUint8();
		name = stream.readString(length);
	}
}

void to_json(ordered_json &j, const ScriptNamesChunk &c) {
	j["unknown0"] = c.unknown0;
	j["unknown1"] = c.unknown1;
	j["len1"] = c.len1;
	j["len2"] = c.len2;
	j["namesOffset"] = c.namesOffset;
	j["namesCount"] = c.namesCount;
	j["names"] = c.names;
}

std::string ScriptNamesChunk::getName(int id) {
	if (-1 < id && (unsigned)id < names.size())
		return names[id];
	return "UNKNOWN_NAME_" + std::to_string(id);
}

}
