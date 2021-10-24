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

struct Rectangle {
	uint16_t top;
	uint16_t left;
	uint16_t bottom;
	uint16_t right;

	void read(Common::ReadStream &stream);
};
void to_json(ordered_json &j, const Rectangle &c);

}

#endif
