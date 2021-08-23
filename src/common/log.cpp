#include <iostream>

#include "common/log.h"

namespace Common {

bool g_verbose = false;

void log(const std::string &msg) {
    std::cout << msg << std::endl;
}

void log(const boost::format &msg) {
    std::cout << msg << std::endl;
}

void debug(const std::string &msg) {
    if (g_verbose)
        log(msg);
}

void debug(const boost::format &msg) {
    if (g_verbose)
        log(msg);
}

}
