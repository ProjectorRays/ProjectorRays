/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_CONTEXT_H
#define LINGODEC_CONTEXT_H

#include <cstdint>
#include <map>
#include <vector>

namespace Common {
class ReadStream;
}

namespace LingoDec {

class ChunkResolver;
struct Script;
struct ScriptContextMapEntry;
struct ScriptNames;

/* ScriptContext */

struct ScriptContext {
	int32_t unknown0;
	int32_t unknown1;
	uint32_t entryCount;
	uint32_t entryCount2;
	uint16_t entriesOffset;
	int16_t unknown2;
	int32_t unknown3;
	int32_t unknown4;
	int32_t unknown5;
	int32_t lnamSectionID;
	uint16_t validCount;
	uint16_t flags;
	int16_t freePointer;

	unsigned int version;
	ChunkResolver *resolver;
	ScriptNames *lnam;
	std::vector<ScriptContextMapEntry> sectionMap;
	std::map<uint32_t, Script *> scripts;

	ScriptContext(unsigned int version, ChunkResolver *resolver) :
		version(version),
		resolver(resolver),
		lnam(nullptr) {}

	void read(Common::ReadStream &stream);
	bool validName(int id) const;
	std::string getName(int id) const;
	void parseScripts();
};

/* ScriptContextMapEntry */

struct ScriptContextMapEntry {
	int32_t unknown0;
	int32_t sectionID;
	uint16_t unknown1;
	uint16_t unknown2;

	void read(Common::ReadStream &stream);
};

} // namespace LingoDec

#endif // LINGODEC_CONTEXT_H
