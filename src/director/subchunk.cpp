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

} // namespace Director
