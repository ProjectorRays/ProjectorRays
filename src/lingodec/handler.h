/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_HANDLER_H
#define LINGODEC_HANDLER_H

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "lingodec/enums.h"

namespace Common {
class CodeWriter;
class JSONWriter;
class ReadStream;
}

namespace LingoDec {

struct AST;
struct Bytecode;
struct Node;
struct Script;

/* Handler */

struct Handler {
	int16_t nameID;
	uint16_t vectorPos;
	uint32_t compiledLen;
	uint32_t compiledOffset;
	uint16_t argumentCount;
	uint32_t argumentOffset;
	uint16_t localsCount;
	uint32_t localsOffset;
	uint16_t globalsCount;
	uint32_t globalsOffset;
	uint32_t unknown1;
	uint16_t unknown2;
	uint16_t lineCount;
	uint32_t lineOffset;
	uint32_t stackHeight;

	std::vector<int16_t> argumentNameIDs;
	std::vector<int16_t> localNameIDs;
	std::vector<int16_t> globalNameIDs;

	Script *script;
	std::vector<Bytecode> bytecodeArray;
	std::map<uint32_t, size_t> bytecodePosMap;
	std::vector<std::string> argumentNames;
	std::vector<std::string> localNames;
	std::vector<std::string> globalNames;
	std::string name;

	std::vector<std::shared_ptr<Node>> stack;
	std::unique_ptr<AST> ast;

	bool isGenericEvent = false;

	Handler(Script *s) {
		script = s;
	}

	void readRecord(Common::ReadStream &stream);
	void readData(Common::ReadStream &stream);
	std::vector<int16_t> readVarnamesTable(Common::ReadStream &stream, uint16_t count, uint32_t offset);
	void readNames();
	bool validName(int id) const;
	std::string getName(int id) const;
	std::string getArgumentName(int id) const;
	std::string getLocalName(int id) const;
	std::shared_ptr<Node> pop();
	int variableMultiplier();
	std::shared_ptr<Node> readVar(int varType);
	std::string getVarNameFromSet(const Bytecode &bytecode);
	std::shared_ptr<Node> readV4Property(int propertyType, int propertyID);
	std::shared_ptr<Node> readChunkRef(std::shared_ptr<Node> string);
	void tagLoops();
	bool isRepeatWithIn(uint32_t startIndex, uint32_t endIndex);
	BytecodeTag identifyLoop(uint32_t startIndex, uint32_t endIndex);
	void parse();
	uint32_t translateBytecode(Bytecode &bytecode, uint32_t index);
	void writeBytecodeText(Common::CodeWriter &code, bool dotSyntax);
};

/* Bytecode */

struct Bytecode {
	uint8_t opID;
	OpCode opcode;
	int32_t obj;
	uint32_t pos;
	BytecodeTag tag;
	uint32_t ownerLoop;
	std::shared_ptr<Node> translation;

	Bytecode(uint8_t op, int32_t o, uint32_t p)
		: opID(op), obj(o), pos(p), tag(kTagNone), ownerLoop(UINT32_MAX) {
		opcode = static_cast<OpCode>(op >= 0x40 ? 0x40 + op % 0x40 : op);
	}
};

}

#endif // LINGODEC_HANDLER_H
