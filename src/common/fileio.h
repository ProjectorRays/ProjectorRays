/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_FILEIO_H
#define COMMON_FILEIO_H

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Common {

class BufferView;

bool readFile(const std::filesystem::path &path, std::vector<uint8_t> &buf);
void writeFile(const std::filesystem::path &path, const std::string &contents);
void writeFile(const std::filesystem::path &path, const uint8_t *contents, size_t size);
void writeFile(const std::filesystem::path &path, const BufferView &view);

} // namespace Common

#endif // COMMON_FILEIO_H
