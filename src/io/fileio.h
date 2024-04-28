/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef IO_FILEIO_H
#define IO_FILEIO_H

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Common {
class BufferView;
}

namespace IO {

#ifdef _WIN32
static const char *kPlatformLineEnding = "\r\n";
#else
static const char *kPlatformLineEnding = "\n";
#endif

bool readFile(const std::filesystem::path &path, std::vector<uint8_t> &buf);

void writeFile(const std::filesystem::path &path, const std::string &contents);
void writeFile(const std::filesystem::path &path, const uint8_t *contents, size_t size);
void writeFile(const std::filesystem::path &path, const Common::BufferView &view);

std::string cleanFileName(const std::string &fileName);

} // namespace IO

#endif // IO_FILEIO_H
