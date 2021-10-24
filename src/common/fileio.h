/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2021 Deborah Servilla
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef COMMON_FILEIO_H
#define COMMON_FILEIO_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Common {

std::shared_ptr<std::vector<uint8_t>> readFile(const std::string &fileName);
void writeFile(const std::string &fileName, const std::string &contents);
void writeFile(const std::string &fileName, const uint8_t *contents, size_t size);

}

#endif
