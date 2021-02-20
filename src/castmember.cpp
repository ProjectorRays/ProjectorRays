#include "castmember.h"
#include "stream.h"

namespace ProjectorRays {

/* CastMember */

void CastMember::read(ReadStream &stream) {}

/* ScriptMember */

void ScriptMember::read(ReadStream &stream) {
    uint16_t rawType = stream.readUint16();
    switch (rawType) {
    case 1:
        scriptType = kScoreScript;
        break;
    case 3:
        scriptType = kMovieScript;
        break;
    case 7:
        scriptType = kParentScript;
        break;
    default:
        scriptType = kUnknownScript;
        break;
    }
}

}
