/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_UTIL_H
#define DIRECTOR_UTIL_H

#include <cstdint>
#include <string>

#define FOURCC(a0,a1,a2,a3) ((uint32_t)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

namespace Director {

std::string fourCCToString(uint32_t fourcc);
std::string indent(std::string str);
unsigned int humanVersion(unsigned int ver);
std::string versionString(unsigned int ver, const std::string &fverVersionString);
std::string cleanFileName(const std::string &fileName);
std::string floatToString(double f);

} // namespace Director

#endif // DIRECTOR_UTIL_H
