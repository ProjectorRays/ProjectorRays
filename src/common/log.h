/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include <string>

namespace Common {

class String;

extern bool g_verbose;

void log(const String &msg);
void debug(const String &msg);

} // namespace Common

void warning(const char *fmt, ...);

#endif // COMMON_LOG_H
