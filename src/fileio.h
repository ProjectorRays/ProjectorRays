#ifndef FILEIO_H
#define FILEIO_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ProjectorRays {

std::shared_ptr<std::vector<uint8_t>> readFile(const std::string &fileName);
void writeFile(const std::string &fileName, const std::string &contents);
void writeFile(const std::string &fileName, const uint8_t *contents, size_t size);

}

#endif
