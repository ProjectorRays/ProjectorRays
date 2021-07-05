#ifndef DIRECTOR_UTIL_H
#define DIRECTOR_UTIL_H

#include <cstdint>
#include <string>

#define FOURCC(a0,a1,a2,a3) ((uint32_t)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

namespace Director {

std::string fourCCToString(uint32_t fourcc);
std::string indent(std::string str);
int humanVersion(int ver);
std::string cleanFileName(const std::string &fileName);
std::string floatToString(double f);

}

#endif
