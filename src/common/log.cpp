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

#include <iostream>

#include "common/log.h"

namespace Common {

bool g_verbose = false;

void log(const std::string &msg) {
	std::cout << msg << std::endl;
}

void log(const boost::format &msg) {
	std::cout << msg << std::endl;
}

void debug(const std::string &msg) {
	if (g_verbose)
		log(msg);
}

void debug(const boost::format &msg) {
	if (g_verbose)
		log(msg);
}

}
