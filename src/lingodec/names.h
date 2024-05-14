/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_NAMES_H
#define LINGODEC_NAMES_H

#include <map>
#include "common/array.h"

namespace Common {
class ReadStream;
class String;
}

namespace LingoDec {

/* StandardNames */

struct StandardNames {
	static std::map<unsigned int, Common::String> opcodeNames;
	static std::map<unsigned int, Common::String> binaryOpNames;
	static std::map<unsigned int, Common::String> chunkTypeNames;
	static std::map<unsigned int, Common::String> putTypeNames;
	static std::map<unsigned int, Common::String> moviePropertyNames;
	static std::map<unsigned int, Common::String> whenEventNames;
	static std::map<unsigned int, Common::String> timeNames;
	static std::map<unsigned int, Common::String> menuPropertyNames;
	static std::map<unsigned int, Common::String> menuItemPropertyNames;
	static std::map<unsigned int, Common::String> soundPropertyNames;
	static std::map<unsigned int, Common::String> spritePropertyNames;
	static std::map<unsigned int, Common::String> animationPropertyNames;
	static std::map<unsigned int, Common::String> animation2PropertyNames;
	static std::map<unsigned int, Common::String> memberPropertyNames;

	static Common::String getOpcodeName(uint8_t id);
	static Common::String getName(const std::map<unsigned int, Common::String> &nameMap, unsigned int id);
};

/* ScriptNames */

struct ScriptNames {
	int32_t unknown0;
	int32_t unknown1;
	uint32_t len1;
	uint32_t len2;
	uint16_t namesOffset;
	uint16_t namesCount;
	Common::Array<Common::String> names;

	unsigned int version;

	ScriptNames(unsigned int version) : version(version) {}
	void read(Common::ReadStream &stream);
	bool validName(int id) const;
	Common::String getName(int id) const;
};

} // namespace LingoDec

#endif // LINGODEC_NAMES_H
