#include <iostream>
#include <vector>

#include "castmember.h"
#include "chunk.h"
#include "fileio.h"
#include "lingo.h"
#include "stream.h"
#include "dirfile.h"
#include "util.h"

using namespace ProjectorRays;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " FILE\n";
        return 1;
    }

    auto buf = readFile(argv[1]);
    auto stream = std::make_unique<ReadStream>(buf);
    auto dir = std::make_unique<DirectorFile>();
    dir->read(stream.get());

    dir->dumpScripts();

    return 0;
}
