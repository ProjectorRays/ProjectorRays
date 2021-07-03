#include "castmember.h"
#include "stream.h"

namespace ProjectorRays {

/* CastMember */

void CastMember::read(ReadStream &stream) {}

/* ScriptMember */

void ScriptMember::read(ReadStream &stream) {
    scriptType = static_cast<ScriptType>(stream.readUint16());
}

}
