/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <format>
#include <memory>
#include <stdarg.h>
#include "common/str.h"

namespace Common {

String String::format(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	int size_s = vsnprintf(nullptr, 0, fmt, va) + 1; // Extra space for '\0'
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	vsnprintf(buf.get(), size, fmt, va);
	va_end(va);
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

String operator+(const String &x, const String &y) {
	return x._str + y._str;
}

String operator+(const char *x, const String &y) {
	return x + y._str;
}

String operator+(const String &x, const char *y) {
	return x._str + y;
}

String operator+(const String &x, char y) {
	return x._str + y;
}

std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os, const String &str) {
	return os << str._str;
}

} // namespace Common
