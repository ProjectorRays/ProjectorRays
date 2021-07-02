#include <iostream>
#include <fstream>

#include "fileio.h"

namespace ProjectorRays {

std::shared_ptr<std::vector<uint8_t>> readFile(const std::string &fileName) {
    std::ifstream f;
    f.open(fileName, std::ios::in | std::ios::binary);
    f.seekg(0, std::ios::end);
    auto fileSize = f.tellg();
    f.seekg(0, std::ios::beg);
    auto buf = std::make_shared<std::vector<uint8_t>>(fileSize, 0);
    f.read((char *)buf->data(), fileSize);
    f.close();
    return buf;
}

void writeFile(const std::string &fileName, const std::string &contents) {
    std::ofstream f;
    f.open(fileName, std::ios::out);
    f << contents;
    f.close();
}

}
