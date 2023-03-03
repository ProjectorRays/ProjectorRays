/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <boost/format.hpp>

#include "common/json.h"
#include "common/log.h"
#include "common/stream.h"
#include "common/util.h"
#include "director/castmember.h"
#include "director/chunk.h"
#include "director/lingo.h"
#include "director/dirfile.h"
#include "director/subchunk.h"
#include "director/util.h"

namespace Director {

/* Chunk */

void Chunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
	json.endObject();
}

/* CastChunk */

void CastChunk::read(Common::ReadStream &stream) {
	stream.endianness = Common::kBigEndian;
	while (!stream.eof()) {
		auto id = stream.readInt32();
		memberIDs.push_back(id);
	}
}

void CastChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		json.writeKey("memberIDs");
		json.startArray();
			for (auto val : memberIDs) {
				json.writeVal(val);
			}
		json.endArray();
	json.endObject();
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

void CastListChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(dataOffset);
		JSON_WRITE_FIELD(unk0);
		JSON_WRITE_FIELD(itemsPerCast);
		JSON_WRITE_FIELD(itemsPerCast);
		JSON_WRITE_FIELD(unk1);
		json.writeKey("entries");
		json.startArray();
			for (const auto &val : entries) {
				val.writeJSON(json);
			}
		json.endArray();
	json.endObject();
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

void CastMemberChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(type);
		JSON_WRITE_FIELD(infoLen);
		if (hasFlags1) {
			JSON_WRITE_FIELD(flags1);
		}
		JSON_WRITE_FIELD(specificDataLen);
		json.writeKey("info");
		info->writeJSON(json);
		json.writeKey("member");
		member->writeJSON(json);
	json.endObject();
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
	if (offsetTableLen == 0) { // Workaround: Increase table len to have at least one entry for decompilation results
		offsetTableLen = 1;
		offsetTable.resize(offsetTableLen);
	}
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

void CastInfoChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(dataOffset);
		JSON_WRITE_FIELD(unk1);
		JSON_WRITE_FIELD(unk2);
		JSON_WRITE_FIELD(flags);
		JSON_WRITE_FIELD(scriptId);
		JSON_WRITE_FIELD(scriptSrcText);
		JSON_WRITE_FIELD(name);
		// JSON_WRITE_FIELD(comment);
		// JSON_WRITE_FIELD(fileFormatID);
		// JSON_WRITE_FIELD(created);
		// JSON_WRITE_FIELD(modified);
	json.endObject();
}

/* ConfigChunk */

void ConfigChunk::read(Common::ReadStream &stream) {
	stream.endianness = Common::kBigEndian;

	stream.seek(36);
	directorVersion = stream.readInt16();
	unsigned int ver = humanVersion(directorVersion);

	stream.seek(0);
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
	if (ver < 700) {
		/* 18 */ preD7field11 = stream.readInt16();
	} else {
		/* 18 */ D7stageColorG = stream.readUint8();
		/* 19 */ D7stageColorB = stream.readUint8();
	}
	/* 20 */ commentFont = stream.readInt16();
	/* 22 */ commentSize = stream.readInt16();
	/* 24 */ commentStyle = stream.readUint16();
	if (ver < 700) {
		/* 26 */ preD7stageColor = stream.readInt16();
	} else {
		/* 26 */ D7stageColorIsRGB = stream.readUint8();
		/* 27 */ D7stageColorR = stream.readUint8();
	}
	/* 28 */ bitDepth = stream.readInt16();
	/* 30 */ field17 = stream.readUint8();
	/* 31 */ field18 = stream.readUint8();
	/* 32 */ field19 = stream.readInt32();
	/* 36 */ /* directorVersion = */ stream.readInt16();
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
	}
}

size_t ConfigChunk::size() {
	return len;
}

void ConfigChunk::write(Common::WriteStream &stream) {
	stream.endianness = Common::kBigEndian;

	unsigned int ver = humanVersion(directorVersion);

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
	if (ver < 700) {
		/* 18 */ stream.writeInt16(preD7field11);
	} else {
		/* 18 */ stream.writeUint8(D7stageColorG);
		/* 19 */ stream.writeUint8(D7stageColorB);
	}
	/* 20 */ stream.writeInt16(commentFont);
	/* 22 */ stream.writeInt16(commentSize);
	/* 24 */ stream.writeUint16(commentStyle);
	if (ver < 700) {
		/* 26 */ stream.writeInt16(preD7stageColor);
	} else {
		/* 26 */ stream.writeUint8(D7stageColorIsRGB);
		/* 27 */ stream.writeUint8(D7stageColorR);
	}
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

	int32_t operand11;
	if (ver < 700) {
		operand11 = preD7field11;
	} else {
		operand11 = dir->endianness
						? (int16_t)((D7stageColorB << 8) | D7stageColorG)
						: (int16_t)((D7stageColorG << 8) | D7stageColorB);
	}
	check += operand11 + 11;
	Common::debug(boost::format("Checksum step 11 (+= %1% + 11): %2%") % operand11 % check);

	check *= commentFont + 12;
	Common::debug(boost::format("Checksum step 12 (*= %1% + 12): %2%") % commentFont % check);

	check += commentSize + 13;
	Common::debug(boost::format("Checksum step 13 (+= %1% + 13): %2%") % commentSize % check);

	int32_t operand14 = (ver < 800) ? (uint8_t)((commentStyle >> 8) & 0xFF) : commentStyle;
	check *= operand14 + 14;
	Common::debug(boost::format("Checksum step 14 (*= %1% + 14): %2%") % operand14 % check);

	int32_t operand15 = (ver < 700) ? preD7stageColor : D7stageColorR;
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

void ConfigChunk::writeJSON(Common::JSONWriter &json) const {
	unsigned int ver = humanVersion(directorVersion);

	json.startObject();
		JSON_WRITE_FIELD(len);
		JSON_WRITE_FIELD(fileVersion);
		JSON_WRITE_FIELD(movieTop);
		JSON_WRITE_FIELD(movieLeft);
		JSON_WRITE_FIELD(movieBottom);
		JSON_WRITE_FIELD(movieRight);
		JSON_WRITE_FIELD(minMember);
		JSON_WRITE_FIELD(maxMember);
		JSON_WRITE_FIELD(field9);
		JSON_WRITE_FIELD(field10);
		if (ver < 700) {
			JSON_WRITE_FIELD(preD7field11);
		} else {
			JSON_WRITE_FIELD(D7stageColorG);
			JSON_WRITE_FIELD(D7stageColorB);
		}
		JSON_WRITE_FIELD(commentFont);
		JSON_WRITE_FIELD(commentSize);
		JSON_WRITE_FIELD(commentStyle);
		if (ver < 700) {
			JSON_WRITE_FIELD(preD7stageColor);
		} else {
			JSON_WRITE_FIELD(D7stageColorIsRGB);
			JSON_WRITE_FIELD(D7stageColorR);
		}
		JSON_WRITE_FIELD(bitDepth);
		JSON_WRITE_FIELD(field17);
		JSON_WRITE_FIELD(field18);
		JSON_WRITE_FIELD(field19);
		JSON_WRITE_FIELD(directorVersion);
		JSON_WRITE_FIELD(field21);
		JSON_WRITE_FIELD(field22);
		JSON_WRITE_FIELD(field23);
		JSON_WRITE_FIELD(field24);
		JSON_WRITE_FIELD(field25);
		JSON_WRITE_FIELD(field26);
		JSON_WRITE_FIELD(frameRate);
		JSON_WRITE_FIELD(platform);
		JSON_WRITE_FIELD(protection);
		JSON_WRITE_FIELD(field29);
		JSON_WRITE_FIELD(checksum);
	json.endObject();
}

/* InitialMapChunk */

void InitialMapChunk::read(Common::ReadStream &stream) {
	version = stream.readUint32();
	mmapOffset = stream.readUint32();
	directorVersion = stream.readUint32();
	unused1 = stream.readUint32();
	unused2 = stream.readUint32();
	unused3 = stream.readUint32();
}

size_t InitialMapChunk::size() {
	return 24;
}

void InitialMapChunk::write(Common::WriteStream &stream) {
	stream.writeUint32(version);
	stream.writeUint32(mmapOffset);
	stream.writeUint32(directorVersion);
	stream.writeUint32(unused1);
	stream.writeUint32(unused2);
	stream.writeUint32(unused3);
}

void InitialMapChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(version);
		JSON_WRITE_FIELD(mmapOffset);
		JSON_WRITE_FIELD(directorVersion);
		JSON_WRITE_FIELD(unused1);
		JSON_WRITE_FIELD(unused2);
		JSON_WRITE_FIELD(unused3);
	json.endObject();
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

void KeyTableChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(entrySize);
		JSON_WRITE_FIELD(entrySize2);
		JSON_WRITE_FIELD(entryCount);
		JSON_WRITE_FIELD(usedCount);
		json.writeKey("entries");
		json.startArray();
			for (const auto &val : entries) {
				val.writeJSON(json);
			}
		json.endArray();
	json.endObject();
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
	headerLength = stream.readInt16();
	entryLength = stream.readInt16();
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
	stream.writeInt16(headerLength);
	stream.writeInt16(entryLength);
	stream.writeInt32(chunkCountMax);
	stream.writeInt32(chunkCountUsed);
	stream.writeInt32(junkHead);
	stream.writeInt32(junkHead2);
	stream.writeInt32(freeHead);
	for (auto &entry : mapArray) {
		entry.write(stream);
	}
}

void MemoryMapChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(headerLength);
		JSON_WRITE_FIELD(entryLength);
		JSON_WRITE_FIELD(chunkCountMax);
		JSON_WRITE_FIELD(chunkCountUsed);
		JSON_WRITE_FIELD(junkHead);
		JSON_WRITE_FIELD(junkHead2);
		JSON_WRITE_FIELD(freeHead);
		json.writeKey("mapArray");
		json.startArray();
			for (const auto &val : mapArray) {
				val.writeJSON(json);
			}
		json.endArray();
	json.endObject();
}

/* ScriptChunk */

ScriptChunk::ScriptChunk(DirectorFile *m) :
	Chunk(m, kScriptChunk),
	context(nullptr),
	member(nullptr) {}

ScriptChunk::~ScriptChunk() = default;

void ScriptChunk::read(Common::ReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	stream.endianness = Common::kBigEndian;

	stream.seek(8);
	/*  8 */ totalLength = stream.readUint32();
	/* 12 */ totalLength2 = stream.readUint32();
	/* 16 */ headerLength = stream.readUint16();
	/* 18 */ scriptNumber = stream.readUint16();
	/* 20 */ unk20 = stream.readInt16();
	/* 22 */ parentNumber = stream.readInt16();
	
	stream.seek(38);
	/* 38 */ scriptFlags = stream.readUint32();
	/* 42 */ unk42 = stream.readInt16();
	/* 44 */ castID = stream.readInt32();
	/* 48 */ factoryNameID = stream.readInt16();
	/* 50 */ handlerVectorsCount = stream.readUint16();
	/* 52 */ handlerVectorsOffset = stream.readUint32();
	/* 56 */ handlerVectorsSize = stream.readUint32();
	/* 60 */ propertiesCount = stream.readUint16();
	/* 62 */ propertiesOffset = stream.readUint32();
	/* 66 */ globalsCount = stream.readUint16();
	/* 68 */ globalsOffset = stream.readUint32();
	/* 72 */ handlersCount = stream.readUint16();
	/* 74 */ handlersOffset = stream.readUint32();
	/* 78 */ literalsCount = stream.readUint16();
	/* 80 */ literalsOffset = stream.readUint32();
	/* 84 */ literalsDataCount = stream.readUint32();
	/* 88 */ literalsDataOffset = stream.readUint32();

	propertyNameIDs = readVarnamesTable(stream, propertiesCount, propertiesOffset);
	globalNameIDs = readVarnamesTable(stream, globalsCount, globalsOffset);

	handlers.resize(handlersCount);
	for (auto &handler : handlers) {
		handler = std::make_unique<Handler>(this);
	}
	if ((scriptFlags & kScriptFlagEventScript) && handlersCount > 0) {
		handlers[0]->isGenericEvent = true;
	}

	stream.seek(handlersOffset);
	for (auto &handler : handlers) {
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

void ScriptChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(totalLength);
		JSON_WRITE_FIELD(totalLength2);
		JSON_WRITE_FIELD(headerLength);
		JSON_WRITE_FIELD(scriptNumber);
		JSON_WRITE_FIELD(unk20);
		JSON_WRITE_FIELD(parentNumber);
		JSON_WRITE_FIELD(scriptFlags);
		JSON_WRITE_FIELD(unk42);
		JSON_WRITE_FIELD(castID);
		JSON_WRITE_FIELD(factoryNameID);
		JSON_WRITE_FIELD(handlerVectorsCount);
		JSON_WRITE_FIELD(handlerVectorsOffset);
		JSON_WRITE_FIELD(handlerVectorsSize);
		JSON_WRITE_FIELD(propertiesCount);
		JSON_WRITE_FIELD(propertiesOffset);
		JSON_WRITE_FIELD(globalsCount);
		JSON_WRITE_FIELD(globalsOffset);
		JSON_WRITE_FIELD(handlersCount);
		JSON_WRITE_FIELD(handlersOffset);
		JSON_WRITE_FIELD(literalsCount);
		JSON_WRITE_FIELD(literalsOffset);
		JSON_WRITE_FIELD(literalsDataCount);
		JSON_WRITE_FIELD(literalsDataOffset);
		json.writeKey("propertyNameIDs");
		json.startArray();
			for (auto val : propertyNameIDs) {
				json.writeVal(val);
			}
		json.endArray();
		json.writeKey("globalNameIDs");
		json.startArray();
			for (auto val : globalNameIDs) {
				json.writeVal(val);
			}
		json.endArray();
		json.writeKey("handlers");
		json.startArray();
			for (const auto &val : handlers) {
				val->writeJSON(json);
			}
		json.endArray();
		json.writeKey("literals");
		json.startArray();
			for (const auto &val : literals) {
				val.writeJSON(json);
			}
		json.endArray();
	json.endObject();
}

bool ScriptChunk::validName(int id) const {
	return context->validName(id);
}

std::string ScriptChunk::getName(int id) const {
	return context->getName(id);
}

void ScriptChunk::setContext(ScriptContextChunk *ctx) {
	this->context = ctx;
	if (factoryNameID != -1) {
		factoryName = getName(factoryNameID);
	}
	for (auto nameID : propertyNameIDs) {
		if (validName(nameID)) {
			std::string name = getName(nameID);
			if (isFactory() && name == "me")
				continue;
			propertyNames.push_back(name);
		}
	}
	for (auto nameID : globalNameIDs) {
		if (validName(nameID)) {
			globalNames.push_back(getName(nameID));
		}
	}
	for (const auto &handler : handlers) {
		handler->readNames();
	}
}

void ScriptChunk::parse() {
	for (const auto &handler : handlers) {
		handler->parse();
	}
}

void ScriptChunk::writeVarDeclarations(Common::CodeWriter &code) const {
	if (!isFactory()) {
		if (propertyNames.size() > 0) {
			code.write("property ");
			for (size_t i = 0; i < propertyNames.size(); i++) {
				if (i > 0)
					code.write(", ");
				code.write(propertyNames[i]);
			}
			code.writeLine();
		}
	}
	if (globalNames.size() > 0) {
		code.write("global ");
		for (size_t i = 0; i < globalNames.size(); i++) {
			if (i > 0)
				code.write(", ");
			code.write(globalNames[i]);
		}
		code.writeLine();
	}
}

void ScriptChunk::writeScriptText(Common::CodeWriter &code) const {
	size_t origSize = code.size();
	writeVarDeclarations(code);
	if (isFactory()) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		code.write("factory ");
		code.writeLine(factoryName);
	}
	for (size_t i = 0; i < handlers.size(); i++) {
		if ((!isFactory() || i > 0) && code.size() != origSize) {
			code.writeLine();
		}
		handlers[i]->ast->writeScriptText(code, dir->dotSyntax, false);
	}
	for (auto factory : factories) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		factory->writeScriptText(code);
	}
}

std::string ScriptChunk::scriptText(const char *lineEnding) const {
	Common::CodeWriter code(lineEnding);
	writeScriptText(code);
	return code.str();
}

void ScriptChunk::writeBytecodeText(Common::CodeWriter &code) const {
	size_t origSize = code.size();
	writeVarDeclarations(code);
	if (isFactory()) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		code.write("factory ");
		code.writeLine(factoryName);
	}
	for (size_t i = 0; i < handlers.size(); i++) {
		if ((!isFactory() || i > 0) && code.size() != origSize) {
			code.writeLine();
		}
		handlers[i]->writeBytecodeText(code);
	}
	for (auto factory : factories) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		factory->writeBytecodeText(code);
	}
}

std::string ScriptChunk::bytecodeText(const char *lineEnding) const {
	Common::CodeWriter code(lineEnding);
	writeBytecodeText(code);
	return code.str();
}

bool ScriptChunk::isFactory() const {
	return (scriptFlags & kScriptFlagFactoryDef);
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
		ScriptChunk *script = it->second.get();
		if (script->isFactory()) {
			ScriptChunk *parent = scripts[script->parentNumber + 1].get();
			parent->factories.push_back(script);
		}
	}
}

void ScriptContextChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(unknown0);
		JSON_WRITE_FIELD(unknown1);
		JSON_WRITE_FIELD(entryCount);
		JSON_WRITE_FIELD(entryCount2);
		JSON_WRITE_FIELD(entriesOffset);
		JSON_WRITE_FIELD(unknown2);
		JSON_WRITE_FIELD(unknown3);
		JSON_WRITE_FIELD(unknown4);
		JSON_WRITE_FIELD(unknown5);
		JSON_WRITE_FIELD(lnamSectionID);
		JSON_WRITE_FIELD(validCount);
		JSON_WRITE_FIELD(flags);
		JSON_WRITE_FIELD(freePointer);
		json.writeKey("sectionMap");
		json.startArray();
			for (const auto &val : sectionMap) {
				val.writeJSON(json);
			}
		json.endArray();
	json.endObject();
}

bool ScriptContextChunk::validName(int id) const {
	return lnam->validName(id);
}

std::string ScriptContextChunk::getName(int id) const {
	return lnam->getName(id);
}

void ScriptContextChunk::parseScripts() {
	for (auto it = scripts.begin(); it != scripts.end(); ++it) {
		it->second->parse();
	}
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

void ScriptNamesChunk::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(unknown0);
		JSON_WRITE_FIELD(unknown1);
		JSON_WRITE_FIELD(len1);
		JSON_WRITE_FIELD(len2);
		JSON_WRITE_FIELD(namesOffset);
		JSON_WRITE_FIELD(namesCount);
		json.writeKey("names");
		json.startArray();
			for (auto val : names) {
				json.writeVal(val);
			}
		json.endArray();
	json.endObject();
}

bool ScriptNamesChunk::validName(int id) const {
	return -1 < id && (unsigned)id < names.size();
}

std::string ScriptNamesChunk::getName(int id) const {
	if (validName(id))
		return names[id];
	return "UNKNOWN_NAME_" + std::to_string(id);
}

} // namespace Director
