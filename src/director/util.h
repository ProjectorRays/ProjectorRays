/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_UTIL_H
#define DIRECTOR_UTIL_H

#include <string>

namespace Director {

std::string indent(std::string str);
unsigned int humanVersion(unsigned int ver);
std::string versionString(unsigned int ver, const std::string &fverVersionString);

} // namespace Director

#endif // DIRECTOR_UTIL_H
