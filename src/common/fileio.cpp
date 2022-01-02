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
#include <fstream>

#include "common/fileio.h"

namespace Common {

std::shared_ptr<std::vector<uint8_t>> readFile(const std::string &fileName) {
	std::ifstream f;
	f.open(fileName, std::ios::in | std::ios::binary);
	f.seekg(0, std::ios::end);
	auto fileSize = f.tellg();
	f.seekg(0, std::ios::beg);
	auto buf = std::make_shared<std::vector<uint8_t>>(fileSize, 0);
	f.read((char *)buf->data(), fileSize);
	f.close();
	return buf;
}

void writeFile(const std::string &fileName, const std::string &contents) {
	std::ofstream f;
	f.open(fileName, std::ios::out);
	f << contents;
	f.close();
}

void writeFile(const std::string &fileName, const uint8_t *contents, size_t size) {
	std::ofstream f;
	f.open(fileName, std::ios::out);
	f.write((char *)contents, size);
	f.close();
}

}
