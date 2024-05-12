/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/stream.h"
#include "common/util.h"
#include "lingodec/context.h"
#include "lingodec/names.h"
#include "lingodec/resolver.h"
#include "lingodec/script.h"

namespace LingoDec {

struct ScriptContextMapEntry;

/* ScriptContext */

void ScriptContext::read(Common::ReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	stream.endianness = Common::kBigEndian;

	unknown0 = stream.readInt32();
	unknown1 = stream.readInt32();
	entryCount = stream.readUint32();
	entryCount2 = stream.readUint32();
	entriesOffset = stream.readUint16();
	unknown2 = stream.readInt16();
	unknown3 = stream.readInt32();
	unknown4 = stream.readInt32();
	unknown5 = stream.readInt32();
	lnamSectionID = stream.readInt32();
	validCount = stream.readUint16();
	flags = stream.readUint16();
	freePointer = stream.readInt16();

	stream.seek(entriesOffset);
	sectionMap.resize(entryCount);
	for (auto &entry : sectionMap) {
		entry.read(stream);
	}

	lnam = resolver->getScriptNames(lnamSectionID);
	for (uint32_t i = 1; i <= entryCount; i++) {
		auto section = sectionMap[i - 1];
		if (section.sectionID > -1) {
			Script *script = resolver->getScript(section.sectionID);
			script->setContext(this);
			scripts[i] = script;
		}
	}

	for (auto it = scripts.begin(); it != scripts.end(); ++it) {
		Script *script = it->second;
		if (script->isFactory()) {
			Script *parent = scripts[script->parentNumber + 1];
			parent->factories.push_back(script);
		}
	}
}

bool ScriptContext::validName(int id) const {
	return lnam->validName(id);
}

std::string ScriptContext::getName(int id) const {
	return lnam->getName(id);
}

void ScriptContext::parseScripts() {
	for (auto it = scripts.begin(); it != scripts.end(); ++it) {
		it->second->parse();
	}
}

/* ScriptContextMapEntry */

void ScriptContextMapEntry::read(Common::ReadStream &stream) {
	unknown0 = stream.readInt32();
	sectionID = stream.readInt32();
	unknown1 = stream.readUint16();
	unknown2 = stream.readUint16();
}

} // namespace LingoDec
