/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "common/stream.h"
#include "director/castmember.h"

namespace Director {

/* CastMember */

void CastMember::read(Common::ReadStream&) {}

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

} // namespace Director
