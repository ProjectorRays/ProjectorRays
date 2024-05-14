/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include <cstdint>
#include <ostream>
#include <string>

typedef unsigned int uint;
typedef uint8_t byte;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;

namespace Common {
class String {
public:
	String() {}
	String(const char *str) : _str(str) {}
	String(std::string s) : _str(s) {}

	inline uint size() const { return _str.size(); }
	inline bool empty() const { return size() == 0; }

	std::string::iterator begin() { return _str.begin(); }
	std::string::iterator end() { return _str.end(); }
	std::string::const_iterator begin() const { return _str.begin(); }
	std::string::const_iterator end() const { return _str.end(); }

	uint32 find(char x, uint32 pos = 0) const { return _str.find(x, pos); }
	static String format(const char *fmt, ...);

	char operator[](int idx) const { return _str[idx]; }
	bool operator==(const char *x) const { return _str == x; }
	String &operator+=(char c) {
		_str += c;
		return *this;
	}
	String &operator+=(const char *str) {
		_str += str;
		return *this;
	}
	String &operator+=(const String &str) {
		_str += str._str;
		return *this;
	}

	const char *c_str() { return _str.c_str(); }

public:
	std::string _str;
	static const uint32 npos = 0xFFFFFFFF;
};

// Append two strings to form a new (temp) string
String operator+(const String &x, const String &y);
String operator+(const char *x, const String &y);
String operator+(const String &x, const char *y);
String operator+(const String &x, char y);

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os, const String &str);
} // namespace Common

#endif
