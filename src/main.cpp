#include <iostream>
#include <vector>

#include "common/fileio.h"
#include "common/stream.h"
#include "director/dirfile.h"

using namespace Director;

int main(int argc, char *argv[]) {
    bool dumpChunks = false;
    std::string fileName;
    bool foundFileName = false;

    int argsUsed;
    for (argsUsed = 1; argsUsed < argc; argsUsed++) {
        std::string arg = argv[argsUsed];
        if (arg == "--dump-chunks") {
            dumpChunks = true;
        } else if (!foundFileName) {
            fileName = arg;
            foundFileName = true;
        } else {
            break;
        }
    }

    if (argsUsed != argc || !foundFileName) {
        std::cout << "Usage: " << argv[0] << " [OPTIONS]... FILE\n";
        std::cout << "  --dump-chunks\t\tDump chunk data\n";
        return 1;
    }

    auto buf = Common::readFile(fileName);
    auto stream = std::make_unique<Common::ReadStream>(buf);
    auto dir = std::make_unique<DirectorFile>();
    dir->read(stream.get());

    dir->dumpScripts();
    if (dumpChunks) {
        dir->dumpChunks();
    }

    return 0;
}
