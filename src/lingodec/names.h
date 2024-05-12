/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_NAMES_H
#define LINGODEC_NAMES_H

#include <map>
#include <string>

namespace Common {
class JSONWriter;
class ReadStream;
}

namespace LingoDec {

/* StandardNames */

struct StandardNames {
	static std::map<unsigned int, std::string> opcodeNames;
	static std::map<unsigned int, std::string> binaryOpNames;
	static std::map<unsigned int, std::string> chunkTypeNames;
	static std::map<unsigned int, std::string> putTypeNames;
	static std::map<unsigned int, std::string> moviePropertyNames;
	static std::map<unsigned int, std::string> whenEventNames;
	static std::map<unsigned int, std::string> timeNames;
	static std::map<unsigned int, std::string> menuPropertyNames;
	static std::map<unsigned int, std::string> menuItemPropertyNames;
	static std::map<unsigned int, std::string> soundPropertyNames;
	static std::map<unsigned int, std::string> spritePropertyNames;
	static std::map<unsigned int, std::string> animationPropertyNames;
	static std::map<unsigned int, std::string> animation2PropertyNames;
	static std::map<unsigned int, std::string> memberPropertyNames;

	static std::string getOpcodeName(uint8_t id);
	static std::string getName(const std::map<unsigned int, std::string> &nameMap, unsigned int id);
};

/* ScriptNames */

struct ScriptNames {
	int32_t unknown0;
	int32_t unknown1;
	uint32_t len1;
	uint32_t len2;
	uint16_t namesOffset;
	uint16_t namesCount;
	std::vector<std::string> names;

	unsigned int version;

	ScriptNames(unsigned int version) : version(version) {}
	void read(Common::ReadStream &stream);
	bool validName(int id) const;
	std::string getName(int id) const;
};

} // namespace LingoDec

#endif // LINGODEC_NAMES_H
