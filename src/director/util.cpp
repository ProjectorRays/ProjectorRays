#include <iomanip>
#include <limits>
#include <sstream>

#include "director/util.h"

namespace Director {

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

int humanVersion(int ver) {
    if (ver >= 0x79F)
        return 1201;
    if (ver >= 0x783)
        return 1200;
    if (ver >= 0x782)
        return 1150;
    if (ver >= 0x781)
        return 1100;
    if (ver >= 0x73B)
        return 1000;
    if (ver >= 0x6A4)
        return 850;
    if (ver >= 0x582)
        return 800;
    if (ver >= 0x4C8)
        return 700;
    if (ver >= 0x4C2)
        return 600;
    if (ver >= 0x4B1)
        return 500;
    if (ver >= 0x45D)
        return 404;
    if (ver >= 0x45B)
        return 400;
    if (ver >= 0x405)
        return 310;
    if (ver >= 0x404)
        return 300;
    return 200;
}

std::string cleanFileName(const std::string &fileName) {
    // Replace any characters that are forbidden in a Windows file name
    // https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file

    std::string res;
    for (char ch : fileName) {
        switch (ch) {
        case '<':
        case '>':
        case ':':
        case '"':
        case '/':
        case '\\':
        case '|':
        case '?':
        case '*':
            res += '_';
            break;
        default:
            res += ch;
            break;
        }
    }
    return res;
}

std::string floatToString(double f) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << f;
    std::string res = ss.str();
    while (res[res.size() - 1] == '0' && res[res.size() - 2] != '.') {
        res.pop_back();
    }
    return res;
}

}
