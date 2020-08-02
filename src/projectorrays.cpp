#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

#include "chunk.h"
#include "lingo.h"
#include "stream.h"
#include "movie.h"
#include "util.h"

using namespace ProjectorRays;

std::shared_ptr<std::vector<uint8_t>> readFile(std::string fileName) {
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

void writeFile(std::string fileName, std::string contents) {
    std::ofstream f;
    f.open(fileName, std::ios::out);
    f << contents;
    f.close();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " [filename]\n";
        return 1;
    }

    auto buf = readFile(argv[1]);
    auto stream = std::make_unique<ReadStream>(buf);
    auto movie = std::make_unique<Movie>();
    movie->read(stream.get());

    for (size_t i = 0; i < movie->scriptContexts.size(); i++) {
        auto lctx = movie->scriptContexts[i];
        for (size_t j = 0; j < lctx->scripts.size(); j++) {
            auto script = lctx->scripts[j];
            auto fileName = "script-" + std::to_string(i) + "-" + std::to_string(j) + ".lingo";
            writeFile(fileName, script->toString());
        }
    }

    return 0;
}
