/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_SUBCHUNK_H
#define DIRECTOR_SUBCHUNK_H

#include <cstdint>
#include <string>

namespace Common {
class JSONWriter;
class ReadStream;
class WriteStream;
}

namespace Director {

struct CastListEntry {
	std::string name;
	std::string filePath;
	uint16_t preloadSettings;
	uint16_t minMember;
	uint16_t maxMember;
	int32_t id;

	void writeJSON(Common::JSONWriter &json) const;
};

struct MemoryMapEntry {
	uint32_t fourCC;
	uint32_t len;
	int32_t offset;
	uint16_t flags;
	int16_t unknown0;
	int32_t next;

	void read(Common::ReadStream &stream);
	void write(Common::WriteStream &stream);
	void writeJSON(Common::JSONWriter &json) const;
};

struct KeyTableEntry {
	int32_t sectionID;
	int32_t castID;
	uint32_t fourCC;

	void read(Common::ReadStream &stream);
	void writeJSON(Common::JSONWriter &json) const;
};

} // namespace Director

#endif // DIRECTOR_SUBCHUNK_H
