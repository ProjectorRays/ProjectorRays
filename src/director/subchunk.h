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

#ifndef DIRECTOR_SUBCHUNK_H
#define DIRECTOR_SUBCHUNK_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

namespace Common {
class ReadStream;
class WriteStream;
}

namespace Director {

struct Datum;

struct CastListEntry {
	std::string name;
	std::string filePath;
	uint16_t preloadSettings;
	uint16_t minMember;
	uint16_t maxMember;
	int32_t id;
};
void to_json(ordered_json &j, const CastListEntry &c);

struct MemoryMapEntry {
	uint32_t fourCC;
	int32_t len;
	int32_t offset;
	uint16_t flags;
	int16_t unknown0;
	int32_t next;

	void read(Common::ReadStream &stream);
	void write(Common::WriteStream &stream);
};
void to_json(ordered_json &j, const MemoryMapEntry &c);

struct ScriptContextMapEntry {
	int32_t unknown0;
	int32_t sectionID;
	uint16_t unknown1;
	uint16_t unknown2;

	void read(Common::ReadStream &stream);
};
void to_json(ordered_json &j, const ScriptContextMapEntry &c);

enum LiteralType {
	kLiteralString	= 1,
	kLiteralInt		= 4,
	kLiteralFloat	= 9
};

struct KeyTableEntry {
	int32_t sectionID;
	int32_t castID;
	uint32_t fourCC;

	void read(Common::ReadStream &stream);
};
void to_json(ordered_json &j, const KeyTableEntry &c);

struct LiteralStore {
	LiteralType type;
	uint32_t offset;
	std::shared_ptr<Datum> value;

	void readRecord(Common::ReadStream &stream, int version);
	void readData(Common::ReadStream &stream, uint32_t startOffset);
};
void to_json(ordered_json &j, const LiteralStore &c);

}

#endif
