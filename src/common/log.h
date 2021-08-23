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

}

#endif
