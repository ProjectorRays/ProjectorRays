/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include <cstdint>
#include <string>

#define FOURCC(a0,a1,a2,a3) ((uint32_t)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)

namespace Common {

std::string fourCCToString(uint32_t fourcc);
std::string floatToString(double f);
std::string byteToString(uint8_t byte);
std::string escapeString(const char *str, size_t size);
std::string escapeString(std::string str);
int stricmp(const char *a, const char *b);
int compareIgnoreCase(const std::string &a, const std::string &b);

} // namespace Common

#endif // COMMON_UTIL_H
