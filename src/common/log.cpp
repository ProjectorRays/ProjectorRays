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

void debug(const Common::String &msg) {
	if (g_verbose)
		log(msg);
}

void warning(const Common::String &msg) {
	std::cerr << msg._str << "\n";
}

} // namespace Common
