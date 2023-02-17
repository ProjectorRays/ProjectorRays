/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

#include <string>
#include <boost/format.hpp>

namespace Common {

extern bool g_verbose;

void log(const std::string &msg);
void log(const boost::format &msg);
void debug(const std::string &msg);
void debug(const boost::format &msg);
void warning(const std::string &msg);
void warning(const boost::format &msg);

}

#endif // COMMON_LOG_H
