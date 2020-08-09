#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <string>

#define FOURCC(a0,a1,a2,a3) ((uint32_t)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

namespace ProjectorRays {

std::string fourCCToString(uint32_t fourcc);
std::string indent(std::string str);
int humanVersion(int ver);

}

#endif
