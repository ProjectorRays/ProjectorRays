/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2021 Deborah Servilla
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "common/stream.h"
#include "director/castmember.h"

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
