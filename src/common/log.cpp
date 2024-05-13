/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include "common/log.h"
#include "common/str.h"

namespace Common {

bool g_verbose = false;

void log(const Common::String &msg) {
	std::cout << msg._str << "\n";
}

void log(const std::string &msg) {
	std::cout << msg << "\n";
}

void log(const boost::format &msg) {
	std::cout << msg << "\n";
}

void debug(const std::string &msg) {
	if (g_verbose)
		log(msg);
}

void debug(const Common::String &msg) {
	if (g_verbose)
		log(msg);
}

void debug(const boost::format &msg) {
	if (g_verbose)
		log(msg);
}

void warning(const std::string &msg) {
	std::cerr << msg << "\n";
}

void warning(const boost::format &msg) {
	std::cerr << msg << "\n";
}

} // namespace Common
