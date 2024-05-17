/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <format>
#include <stdarg.h>
#include <iostream>

#include "common/log.h"
#include "common/str.h"

namespace Common {

bool g_verbose = false;

void log(const Common::String &msg) {
	std::cout << msg._str << "\n";
}

void debug(const Common::String &msg) {
	if (g_verbose)
		log(msg);
}

} // namespace Common

void warning(const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	int size_s = vsnprintf(nullptr, 0, fmt, va) + 1; // Extra space for '\0'
	auto size = static_cast<size_t>(size_s);
	std::unique_ptr<char[]> buf(new char[size]);
	vsnprintf(buf.get(), size, fmt, va);
	va_end(va);
	std::cerr << buf << "\n";
}
