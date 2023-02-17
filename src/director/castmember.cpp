/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/json.h"
#include "common/stream.h"
#include "director/castmember.h"

namespace Director {

/* CastMember */

void CastMember::read(Common::ReadStream&) {}

void CastMember::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
	json.endObject();
}

/* ScriptMember */

void ScriptMember::read(Common::ReadStream &stream) {
	scriptType = static_cast<ScriptType>(stream.readUint16());
}

void ScriptMember::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(scriptType);
	json.endObject();
}

} // namespace Director
