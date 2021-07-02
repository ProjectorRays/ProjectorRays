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

    for (const auto &cast : dir->casts) {
        if (!cast->lctx)
            continue;

        for (auto it = cast->lctx->scripts.begin(); it != cast->lctx->scripts.end(); ++it) {
            std::string scriptType;
            std::string id;
            CastMemberChunk *member = it->second->member;
            if (member) {
                if (member->type == kScriptMember) {
                    ScriptMember *scriptMember = static_cast<ScriptMember *>(member->member.get());
                    switch (scriptMember->scriptType) {
                    case kScoreScript:
                        scriptType = (dir->version >= 600) ? "BehaviorScript" : "ScoreScript";
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
            writeFile(fileName + ".ls", it->second->scriptText());
            writeFile(fileName + ".lasm", it->second->bytecodeText());
        }
    }

    return 0;
}
