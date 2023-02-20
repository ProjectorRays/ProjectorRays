/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>

#include <iomanip>
#include <limits>
#include <sstream>

#include "common/util.h"

namespace Common {

std::string fourCCToString(uint32_t fourcc) {
	char str[4];
	str[0] = (char)(fourcc >> 24);
	str[1] = (char)(fourcc >> 16);
	str[2] = (char)(fourcc >> 8);
	str[3] = (char)fourcc;
	return escapeString(str, sizeof(str));
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

std::string escapeString(const char *str, size_t size) {
	std::string res;
	for (size_t i = 0; i < size; i++) {
		unsigned char ch = str[i];
		switch (ch) {
		case '"':
			res += "\\\"";
			break;
		case '\\':
			res += "\\\\";
			break;
		case '\b':
			res += "\\b";
			break;
		case '\f':
			res += "\\f";
			break;
		case '\n':
			res += "\\n";
			break;
		case '\r':
			res += "\\r";
			break;
		case '\t':
			res += "\\t";
			break;
		case '\v':
			res += "\\v";
			break;
		default:
			if (ch < 0x20 || ch > 0x7f) {
				res += "\\x" + byteToString(ch);
			} else {
				res += ch;
			}
			break;
		}
    }
	return res;
}

std::string escapeString(std::string str) {
	return escapeString(str.c_str(), str.size());
}

} // namespace Common
