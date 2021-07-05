#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "castmember.h"
#include "stream.h"

namespace Director {

/* CastMember */

void CastMember::read(Common::ReadStream &stream) {}

void to_json(ordered_json &j, const CastMember &c) {
    switch (c.type) {
    case kScriptMember:
        to_json(j, static_cast<const ScriptMember &>(c));
        break;
    default:
        break;
    }
}

/* ScriptMember */

void ScriptMember::read(Common::ReadStream &stream) {
    scriptType = static_cast<ScriptType>(stream.readUint16());
}

void to_json(ordered_json &j, const ScriptMember &c) {
    j["scriptType"] = c.scriptType;
}

}
