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

#ifndef COMMON_FILEIO_H
#define COMMON_FILEIO_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace Common {

class BufferView;

std::vector<uint8_t> readFile(const std::string &fileName);
void writeFile(const std::string &fileName, const std::string &contents);
void writeFile(const std::string &fileName, const uint8_t *contents, size_t size);
void writeFile(const std::string &fileName, const BufferView &view);

}

#endif
