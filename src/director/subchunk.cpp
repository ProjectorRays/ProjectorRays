/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/json.h"
#include "common/stream.h"
#include "common/util.h"
#include "director/subchunk.h"
#include "lingodec/ast.h"
#include "lingodec/enums.h"

namespace Director {

/* CastListEntry */

void CastListEntry::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(name);
		JSON_WRITE_FIELD(filePath);
		JSON_WRITE_FIELD(preloadSettings);
		JSON_WRITE_FIELD(minMember);
		JSON_WRITE_FIELD(maxMember);
		JSON_WRITE_FIELD(id);
	json.endObject();
}

/* MemoryMapEntry */

void MemoryMapEntry::read(Common::ReadStream &stream) {
	fourCC = stream.readUint32();
	len = stream.readUint32();
	offset = stream.readUint32();
	flags = stream.readInt16();
	unknown0 = stream.readInt16();
	next = stream.readInt32();
}

void MemoryMapEntry::write(Common::WriteStream &stream) {
	stream.writeUint32(fourCC);
	stream.writeUint32(len);
	stream.writeUint32(offset);
	stream.writeInt16(flags);
	stream.writeInt16(unknown0);
	stream.writeInt32(next);
}

void MemoryMapEntry::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FOURCC_FIELD(fourCC);
		JSON_WRITE_FIELD(len);
		JSON_WRITE_FIELD(offset);
		JSON_WRITE_FIELD(flags);
		JSON_WRITE_FIELD(unknown0);
		JSON_WRITE_FIELD(next);
	json.endObject();
}

/* ScriptContextMapEntry */

void ScriptContextMapEntry::read(Common::ReadStream &stream) {
	unknown0 = stream.readInt32();
	sectionID = stream.readInt32();
	unknown1 = stream.readUint16();
	unknown2 = stream.readUint16();
}

void ScriptContextMapEntry::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(unknown0);
		JSON_WRITE_FIELD(sectionID);
		JSON_WRITE_FIELD(unknown1);
		JSON_WRITE_FIELD(unknown2);
	json.endObject();
}

/* KeyTableEntry */

void KeyTableEntry::read(Common::ReadStream &stream) {
	sectionID = stream.readInt32();
	castID = stream.readInt32();
	fourCC = stream.readUint32();
}

void KeyTableEntry::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(sectionID);
		JSON_WRITE_FIELD(castID);
		JSON_WRITE_FOURCC_FIELD(fourCC);
	json.endObject();
}

/* LiteralStore */

void LiteralStore::readRecord(Common::ReadStream &stream, int version) {
	if (version >= 500)
		type = static_cast<LiteralType>(stream.readUint32());
	else
		type = static_cast<LiteralType>(stream.readUint16());
	offset = stream.readUint32();
}

void LiteralStore::readData(Common::ReadStream &stream, uint32_t startOffset) {
	if (type == kLiteralInt) {
		value = std::make_shared<LingoDec::Datum>((int)offset);
	} else {
		stream.seek(startOffset + offset);
		auto length = stream.readUint32();
		if (type == kLiteralString) {
			value = std::make_shared<LingoDec::Datum>(LingoDec::kDatumString, stream.readString(length - 1));
		} else if (type == kLiteralFloat) {
			double floatVal = 0.0;
			if (length == 8) {
				floatVal = stream.readDouble();
			} else if (length == 10) {
				floatVal = stream.readAppleFloat80();
			}
			value = std::make_shared<LingoDec::Datum>(floatVal);
		} else {
			value = std::make_shared<LingoDec::Datum>();
		}
	}
}

void LiteralStore::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(type);
		JSON_WRITE_FIELD(offset);
		json.writeKey("value");
		value->writeJSON(json);
	json.endObject();
}

} // namespace Director
