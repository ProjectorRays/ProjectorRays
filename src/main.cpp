#include <iostream>
#include <vector>

#include "common/fileio.h"
#include "common/log.h"
#include "common/stream.h"
#include "director/dirfile.h"

using namespace Director;

int main(int argc, char *argv[]) {
    bool decompile = true;
    bool dumpChunks = false;
    std::string fileName;
    bool foundFileName = false;

    int argsUsed;
    for (argsUsed = 1; argsUsed < argc; argsUsed++) {
        std::string arg = argv[argsUsed];
        if (arg == "--dump-chunks") {
            dumpChunks = true;
        } else if (arg == "--no-decompile") {
            decompile = false;
        } else if (arg == "-v" || arg == "--verbose") {
            Common::g_verbose = true;
        } else if (!foundFileName) {
            fileName = arg;
            foundFileName = true;
        } else {
            break;
        }
    }

    if (argsUsed != argc || !foundFileName) {
        Common::log(boost::format("Usage: %s [OPTIONS]... FILE") % argv[0]);
        Common::log("  --dump-chunks\t\tDump chunk data");
        Common::log("  --no-decompile\tDon't decompile Lingo");
        Common::log("  -v or --verbose\tVerbose logging");
        return 1;
    }

    auto buf = Common::readFile(fileName);
    auto stream = std::make_unique<Common::ReadStream>(buf);
    auto dir = std::make_unique<DirectorFile>();
    dir->read(stream.get(), decompile);

    if (decompile) {
        dir->dumpScripts();
    }
    if (dumpChunks) {
        dir->dumpChunks();
    }

    return 0;
}
