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

#include <iomanip>
#include <limits>
#include <sstream>

#include "director/lingo.h"
#include "director/util.h"

namespace Director {

std::string fourCCToString(uint32_t fourcc) {
	char str[5];
	str[0] = (char)(fourcc >> 24);
	str[1] = (char)(fourcc >> 16);
	str[2] = (char)(fourcc >> 8);
	str[3] = (char)fourcc;
	str[4] = '\0';
	return std::string(str);
}

std::string indent(std::string str) {
	std::string res;
	size_t pos = str.find(kLingoLineEnding);
	while (pos != std::string::npos) {
		res += "  " + str.substr(0, pos + 1);
		str = str.substr(pos + 1);
		pos = str.find(kLingoLineEnding);
	}
	return res;
}

unsigned int humanVersion(unsigned int ver) {
	if (ver >= 0x79F)
		return 1201;
	if (ver >= 0x783)
		return 1200;
	if (ver >= 0x782)
		return 1150;
	if (ver >= 0x781)
		return 1100;
	if (ver >= 0x73B)
		return 1000;
	if (ver >= 0x6A4)
		return 850;
	if (ver >= 0x582)
		return 800;
	if (ver >= 0x4C8)
		return 700;
	if (ver >= 0x4C2)
		return 600;
	if (ver >= 0x4B1)
		return 500;
	if (ver >= 0x45D)
		return 404;
	if (ver >= 0x45B)
		return 400;
	if (ver >= 0x405)
		return 310;
	if (ver >= 0x404)
		return 300;
	return 200;
}

std::string versionString(unsigned int ver, const std::string &fverVersionString) {
	unsigned int major = ver / 100;
	unsigned int minor = (ver / 10) % 10;
	unsigned int patch = ver % 10;

	std::string versionNumber;
	if (fverVersionString.empty()) {
		versionNumber = std::to_string(major) + "." + std::to_string(minor);
		if (patch)
			versionNumber += "." + std::to_string(patch);
	} else {
		versionNumber = fverVersionString;
	}

	if (major >= 11)
		return "Adobe Director " + versionNumber;

	if (major == 10)
		return "Macromedia Director MX 2004 (" + versionNumber + ")";

	if (major == 9)
		return "Macromedia Director MX (" + versionNumber + ")";

	return "Macromedia Director " + versionNumber;
}

std::string cleanFileName(const std::string &fileName) {
	// Replace any characters that are forbidden in a Windows file name
	// https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file

	std::string res;
	for (char ch : fileName) {
		switch (ch) {
		case '<':
		case '>':
		case ':':
		case '"':
		case '/':
		case '\\':
		case '|':
		case '?':
		case '*':
			res += '_';
			break;
		default:
			res += ch;
			break;
		}
	}
	return res;
}

std::string floatToString(double f) {
	std::stringstream ss;
	ss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << f;
	std::string res = ss.str();
	while (res[res.size() - 1] == '0' && res[res.size() - 2] != '.') {
		res.pop_back();
	}
	return res;
}

}
