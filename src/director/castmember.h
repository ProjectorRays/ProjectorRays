#ifndef DIRECTOR_CASTMEMBER_H
#define DIRECTOR_CASTMEMBER_H

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

namespace Common {
class ReadStream;
}

namespace Director {

class DirectorFile;

enum MemberType {
    kNullMember         = 0,
    kBitmapMember       = 1,
    kFilmLoopMember     = 2,
    kTextMember         = 3,
    kPaletteMember      = 4,
    kPictureMember      = 5,
    kSoundMember        = 6,
    kButtonMember       = 7,
    kShapeMember        = 8,
    kMovieMember        = 9,
    kDigitalVideoMember = 10,
    kScriptMember       = 11,
    kRTEMember          = 12
};

struct CastMember {
    DirectorFile *dir;
    MemberType type;

    CastMember(DirectorFile *d, MemberType t) : dir(d), type(t) {}
    virtual ~CastMember() = default;
    virtual void read(Common::ReadStream &stream);
};
void to_json(ordered_json &j, const CastMember &c);

enum ScriptType {
    kScoreScript = 1,
    kMovieScript = 3,
    kParentScript = 7
};

struct ScriptMember : CastMember {
    ScriptType scriptType;

    ScriptMember(DirectorFile *m) : CastMember(m, kScriptMember) {}
    virtual ~ScriptMember() = default;
    virtual void read(Common::ReadStream &stream);
};
void to_json(ordered_json &j, const ScriptMember &c);

}

#endif
