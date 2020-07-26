#include "util.h"

namespace ProjectorRays {

std::string fourCCToString(uint32_t fourcc) {
    char str[5];
    str[0] = (char)(fourcc >> 24);
    str[1] = (char)(fourcc >> 16);
    str[2] = (char)(fourcc >> 8);
    str[3] = (char)fourcc;
    str[4] = '\0';
    return std::string(str);
}

std::string indent(std::string str) {
    std::string res;
    size_t pos = str.find("\n");
    while (pos != std::string::npos) {
        res += "  " + str.substr(0, pos + 1);
        str = str.substr(pos + 1);
        pos = str.find("\n");
    }
    return res;
}

}
