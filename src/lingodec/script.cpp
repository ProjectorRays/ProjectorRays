/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/codewriter.h"
#include "common/stream.h"
#include "lingodec/ast.h"
#include "lingodec/context.h"
#include "lingodec/handler.h"
#include "lingodec/script.h"

namespace LingoDec {

/* Script */

Script::Script(unsigned int version) :
	version(version),
	context(nullptr) {}

Script::~Script() = default;

void Script::read(Common::ReadStream &stream) {
	// Lingo scripts are always big endian regardless of file endianness
	stream.endianness = Common::kBigEndian;

	stream.seek(8);
	/*  8 */ totalLength = stream.readUint32();
	/* 12 */ totalLength2 = stream.readUint32();
	/* 16 */ headerLength = stream.readUint16();
	/* 18 */ scriptNumber = stream.readUint16();
	/* 20 */ unk20 = stream.readInt16();
	/* 22 */ parentNumber = stream.readInt16();
	
	stream.seek(38);
	/* 38 */ scriptFlags = stream.readUint32();
	/* 42 */ unk42 = stream.readInt16();
	/* 44 */ castID = stream.readInt32();
	/* 48 */ factoryNameID = stream.readInt16();
	/* 50 */ handlerVectorsCount = stream.readUint16();
	/* 52 */ handlerVectorsOffset = stream.readUint32();
	/* 56 */ handlerVectorsSize = stream.readUint32();
	/* 60 */ propertiesCount = stream.readUint16();
	/* 62 */ propertiesOffset = stream.readUint32();
	/* 66 */ globalsCount = stream.readUint16();
	/* 68 */ globalsOffset = stream.readUint32();
	/* 72 */ handlersCount = stream.readUint16();
	/* 74 */ handlersOffset = stream.readUint32();
	/* 78 */ literalsCount = stream.readUint16();
	/* 80 */ literalsOffset = stream.readUint32();
	/* 84 */ literalsDataCount = stream.readUint32();
	/* 88 */ literalsDataOffset = stream.readUint32();

	propertyNameIDs = readVarnamesTable(stream, propertiesCount, propertiesOffset);
	globalNameIDs = readVarnamesTable(stream, globalsCount, globalsOffset);

	handlers.resize(handlersCount);
	for (auto &handler : handlers) {
		handler = std::make_unique<LingoDec::Handler>(this);
	}
	if ((scriptFlags & LingoDec::kScriptFlagEventScript) && handlersCount > 0) {
		handlers[0]->isGenericEvent = true;
	}

	stream.seek(handlersOffset);
	for (auto &handler : handlers) {
		handler->readRecord(stream);
	}
	for (const auto &handler : handlers) {
		handler->readData(stream);
	}

	stream.seek(literalsOffset);
	literals.resize(literalsCount);
	for (auto &literal : literals) {
		literal.readRecord(stream, version);
	}
	for (auto &literal : literals) {
		literal.readData(stream, literalsDataOffset);
	}
}

std::vector<int16_t> Script::readVarnamesTable(Common::ReadStream &stream, uint16_t count, uint32_t offset) {
	stream.seek(offset);
	std::vector<int16_t> nameIDs(count);
	for (uint16_t i = 0; i < count; i++) {
		nameIDs[i] = stream.readInt16();
	}
	return nameIDs;
}

bool Script::validName(int id) const {
	return context->validName(id);
}

std::string Script::getName(int id) const {
	return context->getName(id);
}

void Script::setContext(ScriptContext *ctx) {
	this->context = ctx;
	if (factoryNameID != -1) {
		factoryName = getName(factoryNameID);
	}
	for (auto nameID : propertyNameIDs) {
		if (validName(nameID)) {
			std::string name = getName(nameID);
			if (isFactory() && name == "me")
				continue;
			propertyNames.push_back(name);
		}
	}
	for (auto nameID : globalNameIDs) {
		if (validName(nameID)) {
			globalNames.push_back(getName(nameID));
		}
	}
	for (const auto &handler : handlers) {
		handler->readNames();
	}
}

void Script::parse() {
	for (const auto &handler : handlers) {
		handler->parse();
	}
}

void Script::writeVarDeclarations(Common::CodeWriter &code) const {
	if (!isFactory()) {
		if (propertyNames.size() > 0) {
			code.write("property ");
			for (size_t i = 0; i < propertyNames.size(); i++) {
				if (i > 0)
					code.write(", ");
				code.write(propertyNames[i]);
			}
			code.writeLine();
		}
	}
	if (globalNames.size() > 0) {
		code.write("global ");
		for (size_t i = 0; i < globalNames.size(); i++) {
			if (i > 0)
				code.write(", ");
			code.write(globalNames[i]);
		}
		code.writeLine();
	}
}

void Script::writeScriptText(Common::CodeWriter &code, bool dotSyntax) const {
	size_t origSize = code.size();
	writeVarDeclarations(code);
	if (isFactory()) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		code.write("factory ");
		code.writeLine(factoryName);
	}
	for (size_t i = 0; i < handlers.size(); i++) {
		if ((!isFactory() || i > 0) && code.size() != origSize) {
			code.writeLine();
		}
		handlers[i]->ast->writeScriptText(code, dotSyntax, false);
	}
	for (auto factory : factories) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		factory->writeScriptText(code, dotSyntax);
	}
}

std::string Script::scriptText(const char *lineEnding, bool dotSyntax) const {
	Common::CodeWriter code(lineEnding);
	writeScriptText(code, dotSyntax);
	return code.str();
}

void Script::writeBytecodeText(Common::CodeWriter &code, bool dotSyntax) const {
	size_t origSize = code.size();
	writeVarDeclarations(code);
	if (isFactory()) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		code.write("factory ");
		code.writeLine(factoryName);
	}
	for (size_t i = 0; i < handlers.size(); i++) {
		if ((!isFactory() || i > 0) && code.size() != origSize) {
			code.writeLine();
		}
		handlers[i]->writeBytecodeText(code, dotSyntax);
	}
	for (auto factory : factories) {
		if (code.size() != origSize) {
			code.writeLine();
		}
		factory->writeBytecodeText(code, dotSyntax);
	}
}

std::string Script::bytecodeText(const char *lineEnding, bool dotSyntax) const {
	Common::CodeWriter code(lineEnding);
	writeBytecodeText(code, dotSyntax);
	return code.str();
}

bool Script::isFactory() const {
	return (scriptFlags & LingoDec::kScriptFlagFactoryDef);
}

/* LiteralStore */

void LiteralStore::readRecord(Common::ReadStream &stream, int version) {
	if (version >= 500)
		type = static_cast<LiteralType>(stream.readUint32());
	else
		type = static_cast<LiteralType>(stream.readUint16());
	offset = stream.readUint32();
}

void LiteralStore::readData(Common::ReadStream &stream, uint32_t startOffset) {
	if (type == kLiteralInt) {
		value = std::make_shared<LingoDec::Datum>((int)offset);
	} else {
		stream.seek(startOffset + offset);
		auto length = stream.readUint32();
		if (type == kLiteralString) {
			value = std::make_shared<LingoDec::Datum>(LingoDec::kDatumString, stream.readString(length - 1));
		} else if (type == kLiteralFloat) {
			double floatVal = 0.0;
			if (length == 8) {
				floatVal = stream.readDouble();
			} else if (length == 10) {
				floatVal = stream.readAppleFloat80();
			}
			value = std::make_shared<LingoDec::Datum>(floatVal);
		} else {
			value = std::make_shared<LingoDec::Datum>();
		}
	}
}

} // namespace LingoDec
