#include <cstdint>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>

#include "castmember.h"
#include "chunk.h"
#include "lingo.h"
#include "stream.h"
#include "movie.h"
#include "util.h"

using namespace ProjectorRays;

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " FILE\n";
        return 1;
    }

    auto buf = readFile(argv[1]);
    auto stream = std::make_unique<ReadStream>(buf);
    auto movie = std::make_unique<Movie>();
    movie->read(stream.get());

    for (const auto &cast : movie->casts) {
        for (auto it = cast->lctx->scripts.begin(); it != cast->lctx->scripts.end(); ++it) {
            std::string scriptType;
            std::string id;
            CastMemberChunk *member = it->second->member;
            if (member) {
                if (member->type == kScriptMember) {
                    ScriptMember *scriptMember = static_cast<ScriptMember *>(member->member.get());
                    switch (scriptMember->scriptType) {
                    case kScoreScript:
                        scriptType = (movie->version >= 600) ? "BehaviorScript" : "ScoreScript";
                        break;
                    case kMovieScript:
                        scriptType = "MovieScript";
                        break;
                    case kParentScript:
                        scriptType = "ParentScript";
                        break;
                    default:
                        scriptType = "UnknownScript";
                        break;
                    }
                } else {
                    scriptType = "CastScript";
                }
                id = member->info->name.empty()
                    ? std::to_string(member->id)
                    : member->info->name;
            } else {
                scriptType = "UnknownScript";
                id = std::to_string(it->first);
            }
            std::string fileName = cleanFileName("Cast " + cast->name + " " + scriptType + " " + id);
            writeFile(fileName + ".lingo", it->second->scriptText());
            writeFile(fileName + ".lbc", it->second->bytecodeText());
        }
    }

    return 0;
}
