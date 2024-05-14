/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_SCRIPT_H
#define LINGODEC_SCRIPT_H

#include <cstdint>

#include "common/ptr.h"
#include "common/str.h"
#include "lingodec/enums.h"

namespace Common {
class CodeWriter;
class ReadStream;
}

namespace LingoDec {

struct Datum;
struct Handler;
struct ScriptContext;

/* LiteralStore */

struct LiteralStore {
	LiteralType type;
	uint32_t offset;
	Common::SharedPtr<Datum> value;

	void readRecord(Common::ReadStream &stream, int version);
	void readData(Common::ReadStream &stream, uint32_t startOffset);
};

/* Script */

struct Script {
	/*  8 */ uint32_t totalLength;
	/* 12 */ uint32_t totalLength2;
	/* 16 */ uint16_t headerLength;
	/* 18 */ uint16_t scriptNumber;
	/* 20 */ int16_t unk20;
	/* 22 */ int16_t parentNumber;

	/* 38 */ uint32_t scriptFlags;
	/* 42 */ int16_t unk42;
	/* 44 */ int32_t castID;
	/* 48 */ int16_t factoryNameID;
	/* 50 */ uint16_t handlerVectorsCount;
	/* 52 */ uint32_t handlerVectorsOffset;
	/* 56 */ uint32_t handlerVectorsSize;
	/* 60 */ uint16_t propertiesCount;
	/* 62 */ uint32_t propertiesOffset;
	/* 66 */ uint16_t globalsCount;
	/* 68 */ uint32_t globalsOffset;
	/* 72 */ uint16_t handlersCount;
	/* 74 */ uint32_t handlersOffset;
	/* 78 */ uint16_t literalsCount;
	/* 80 */ uint32_t literalsOffset;
	/* 84 */ uint32_t literalsDataCount;
	/* 88 */ uint32_t literalsDataOffset;

	Common::Array<int16_t> propertyNameIDs;
	Common::Array<int16_t> globalNameIDs;

	Common::String factoryName;
	Common::Array<Common::String> propertyNames;
	Common::Array<Common::String> globalNames;
	Common::Array<Common::ScopedPtr<Handler>> handlers;
	Common::Array<LiteralStore> literals;
	Common::Array<Script *> factories;

	unsigned int version;
	ScriptContext *context;

	Script(unsigned int version);
	~Script();
	void read(Common::ReadStream &stream);
	Common::Array<int16_t> readVarnamesTable(Common::ReadStream &stream, uint16_t count, uint32_t offset);
	bool validName(int id) const;
	Common::String getName(int id) const;
	void setContext(ScriptContext *ctx);
	void parse();
	void writeVarDeclarations(Common::CodeWriter &code) const;
	void writeScriptText(Common::CodeWriter &code, bool dotSyntax) const;
	Common::String scriptText(const char *lineEnding, bool dotSyntax) const;
	void writeBytecodeText(Common::CodeWriter &code, bool dotSyntax) const;
	Common::String bytecodeText(const char *lineEnding, bool dotSyntax) const;

	bool isFactory() const;
};

} // namespace LingoDec

#endif // LINGODEC_SCRIPT_H
