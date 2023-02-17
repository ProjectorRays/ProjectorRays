/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <iomanip>
#include <limits>
#include <sstream>

#include "common/util.h"

std::string fourCCToString(uint32_t fourcc) {
	char str[5];
	str[0] = (char)(fourcc >> 24);
	str[1] = (char)(fourcc >> 16);
	str[2] = (char)(fourcc >> 8);
	str[3] = (char)fourcc;
	str[4] = '\0';
	return std::string(str);
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

std::string byteToString(uint8_t byte) {
	char hex[3];
	snprintf(hex, sizeof(hex), "%02X", byte);
	return std::string(hex);
}
