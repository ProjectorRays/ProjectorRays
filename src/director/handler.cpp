/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "common/json.h"
#include "common/log.h"
#include "common/stream.h"
#include "common/util.h"
#include "director/chunk.h"
#include "director/lingo.h"
#include "director/dirfile.h"

namespace Director {

/* Handler */

void Handler::readRecord(Common::ReadStream &stream) {
	nameID = stream.readInt16();
	vectorPos = stream.readUint16();
	compiledLen = stream.readUint32();
	compiledOffset = stream.readUint32();
	argumentCount = stream.readUint16();
	argumentOffset = stream.readUint32();
	localsCount = stream.readUint16();
	localsOffset = stream.readUint32();
	globalsCount = stream.readUint16();
	globalsOffset = stream.readUint32();
	unknown1 = stream.readUint32();
	unknown2 = stream.readUint16();
	lineCount = stream.readUint16();
	lineOffset = stream.readUint32();
	// yet to implement
	if (script->dir->capitalX)
		stackHeight = stream.readUint32();
}

void Handler::writeJSON(Common::JSONWriter &json) const {
	json.startObject();
		JSON_WRITE_FIELD(nameID);
		JSON_WRITE_FIELD(vectorPos);
		JSON_WRITE_FIELD(compiledLen);
		JSON_WRITE_FIELD(compiledOffset);
		JSON_WRITE_FIELD(argumentCount);
		JSON_WRITE_FIELD(argumentOffset);
		JSON_WRITE_FIELD(localsCount);
		JSON_WRITE_FIELD(localsOffset);
		JSON_WRITE_FIELD(globalsCount);
		JSON_WRITE_FIELD(globalsOffset);
		JSON_WRITE_FIELD(unknown1);
		JSON_WRITE_FIELD(unknown2);
		JSON_WRITE_FIELD(lineCount);
		JSON_WRITE_FIELD(lineOffset);
		if (script->dir->capitalX) {
			JSON_WRITE_FIELD(stackHeight);
		}
	json.endObject();
}

void Handler::readData(Common::ReadStream &stream) {
	stream.seek(compiledOffset);
	while (stream.pos() < compiledOffset + compiledLen) {
		uint32_t pos = stream.pos() - compiledOffset;
		uint8_t op = stream.readUint8();
		OpCode opcode = static_cast<OpCode>(op >= 0x40 ? 0x40 + op % 0x40 : op);
		// argument can be one, two or four bytes
		int32_t obj = 0;
		if (op >= 0xc0) {
			// four bytes
			obj = stream.readInt32();
		} else if (op >= 0x80) {
			// two bytes
			if (opcode == kOpPushInt16 || opcode == kOpPushInt8) {
				// treat pushint's arg as signed
				// pushint8 may be used to push a 16-bit int in older Lingo
				obj = stream.readInt16();
			} else {
				obj = stream.readUint16();
			}
		} else if (op >= 0x40) {
			// one byte
			if (opcode == kOpPushInt8) {
				// treat pushint's arg as signed
				obj = stream.readInt8();
			} else {
				obj = stream.readUint8();
			}
		}
		Bytecode bytecode(op, obj, pos);
		bytecodeArray.push_back(bytecode);
		bytecodePosMap[pos] = bytecodeArray.size() - 1;
	}

	argumentNameIDs = readVarnamesTable(stream, argumentCount, argumentOffset);
	localNameIDs = readVarnamesTable(stream, localsCount, localsOffset);
	globalNameIDs = readVarnamesTable(stream, globalsCount, globalsOffset);
}

std::vector<int16_t> Handler::readVarnamesTable(Common::ReadStream &stream, uint16_t count, uint32_t offset) {
	stream.seek(offset);
	std::vector<int16_t> nameIDs;
	nameIDs.resize(count);
	for (size_t i = 0; i < count; i++) {
		nameIDs[i] = stream.readUint16();
	}
	return nameIDs;
}

void Handler::readNames() {
	name = getName(nameID);
	for (auto nameID : argumentNameIDs) {
		argumentNames.push_back(getName(nameID));
	}
	for (auto nameID : localNameIDs) {
		localNames.push_back(getName(nameID));
	}
	for (auto nameID : globalNameIDs) {
		if (nameID >= 0) // Some global nameIDs = -1 might exist
			globalNames.push_back(getName(nameID));
	}
}

std::string Handler::getName(int id) {
	return script->getName(id);
}

std::string Handler::getArgumentName(int id) {
	if (-1 < id && (unsigned)id < argumentNames.size())
		return argumentNames[id];
	return "UNKNOWN_ARG_" + std::to_string(id);
}

std::string Handler::getLocalName(int id) {
	if (-1 < id && (unsigned)id < localNames.size())
		return localNames[id];
	return "UNKNOWN_LOCAL_" + std::to_string(id);
}

std::string Handler::getGlobalName(int id) {
	if (-1 < id && (unsigned)id < globalNames.size())
		return globalNames[id];
	return "UNKNOWN_GLOBAL_" + std::to_string(id);
}

std::shared_ptr<Node> Handler::peek() {
	if (stack.empty())
		return std::make_shared<ErrorNode>();

	auto res = stack.back();
	return res;
}

std::shared_ptr<Node> Handler::pop() {
	if (stack.empty())
		return std::make_shared<ErrorNode>();

	auto res = stack.back();
	stack.pop_back();
	return res;
}

int Handler::variableMultiplier() {
	// TODO: Determine what version this changed to 1.
	// For now approximating it with the point at which Lctx changed to LctX.
	if (script->dir->capitalX)
		return 1;
	if (script->dir->version >= 500)
		return 8;
	return 6;
}

std::shared_ptr<Node> Handler::readVar(int varType) {
	std::shared_ptr<Node> castID;
	if (varType == 0x6 && script->dir->version >= 500) // field cast ID
		castID = pop();
	std::shared_ptr<Node> id = pop();

	switch (varType) {
	case 0x1: // global
	case 0x2: // global
	case 0x3: // property/instance
		return id;
	case 0x4: // arg
		{
			std::string name = getArgumentName(id->getValue()->i / variableMultiplier());
			auto ref = std::make_shared<Datum>(kDatumVarRef, name);
			return std::make_shared<LiteralNode>(std::move(ref));
		}
	case 0x5: // local
		{
			std::string name = getLocalName(id->getValue()->i / variableMultiplier());
			auto ref = std::make_shared<Datum>(kDatumVarRef, name);
			return std::make_shared<LiteralNode>(std::move(ref));
		}
	case 0x6: // field
		return std::make_shared<MemberExprNode>("field", std::move(id), std::move(castID));
	default:
		Common::warning(boost::format("findVar: unhandled var type %d") % varType);
		break;
	}
	return std::make_shared<ErrorNode>();
}

std::string Handler::getVarNameFromSet(const Bytecode &bytecode) {
	std::string varName;
	switch (bytecode.opcode) {
	case kOpSetGlobal:
	case kOpSetGlobal2:
		varName = getName(bytecode.obj);
		break;
	case kOpSetProp:
		varName = getName(bytecode.obj);
		break;
	case kOpSetParam:
		varName = getArgumentName(bytecode.obj / variableMultiplier());
		break;
	case kOpSetLocal:
		varName = getLocalName(bytecode.obj / variableMultiplier());
		break;
	default:
		varName = "ERROR";
		break;
	}
	return varName;
}

std::shared_ptr<Node> Handler::readV4Property(int propertyType, int propertyID) {
	switch (propertyType) {
	case 0x00:
		{
			if (propertyID <= 0x0b) { // movie property
				auto propName = Lingo::getName(Lingo::moviePropertyNames, propertyID);
				return std::make_shared<TheExprNode>(propName);
			} else { // last chunk
				auto string = pop();
				auto chunkType = static_cast<ChunkExprType>(propertyID - 0x0b);
				return std::make_shared<LastStringChunkExprNode>(chunkType, std::move(string));
			}
		}
		break;
	case 0x01: // number of chunks
		{
			auto string = pop();
			return std::make_shared<StringChunkCountExprNode>(static_cast<ChunkExprType>(propertyID), std::move(string));
		}
		break;
	case 0x02: // menu property
		{
			auto menuID = pop();
			return std::make_shared<MenuPropExprNode>(std::move(menuID), propertyID);
		}
		break;
	case 0x03: // menu item property
		{
			auto menuID = pop();
			auto itemID = pop();
			return std::make_shared<MenuItemPropExprNode>(std::move(menuID), std::move(itemID), propertyID);
		}
		break;
	case 0x04: // sound property
		{
			auto soundID = pop();
			return std::make_shared<SoundPropExprNode>(std::move(soundID), propertyID);
		}
		break;
	case 0x05: // resource property - unused?
		return std::make_shared<CommentNode>("ERROR: Resource property");
	case 0x06: // sprite property
		{
			auto spriteID = pop();
			return std::make_shared<SpritePropExprNode>(std::move(spriteID), propertyID);
		}
		break;
	case 0x07: // animation property
		return std::make_shared<TheExprNode>(Lingo::getName(Lingo::animationPropertyNames, propertyID));
	case 0x08: // animation 2 property
		if (propertyID == 0x02 && script->dir->version >= 500) { // the number of castMembers supports castLib selection from Director 5.0
			auto castLib = pop();
			if (!(castLib->type == kLiteralNode && castLib->getValue()->type == kDatumInt && castLib->getValue()->toInt() == 0)) {
				auto castLibNode = std::make_shared<MemberExprNode>("castLib", castLib, nullptr);
				return std::make_shared<ThePropExprNode>(castLibNode, Lingo::getName(Lingo::animation2PropertyNames, propertyID));
			}
		}
		return std::make_shared<TheExprNode>(Lingo::getName(Lingo::animation2PropertyNames, propertyID));
	case 0x09: // generic cast member
	case 0x0a: // chunk of cast member
	case 0x0b: // field
	case 0x0c: // chunk of field
	case 0x0d: // digital video
	case 0x0e: // bitmap
	case 0x0f: // sound
	case 0x10: // button
	case 0x11: // shape
	case 0x12: // movie
	case 0x13: // script
	case 0x14: // scriptText
	case 0x15: // chunk of scriptText
		{
			auto propName = Lingo::getName(Lingo::memberPropertyNames, propertyID);
			std::shared_ptr<Node> castID;
			if (script->dir->version >= 500) {
				castID = pop();
			}
			auto memberID = pop();
			std::string prefix;
			if (propertyType == 0x0b || propertyType == 0x0c) {
				prefix = "field";
			} else if (propertyType == 0x14 || propertyType == 0x15) {
				prefix = "script";
			} else {
				prefix = (script->dir->version >= 500) ? "member" : "cast";
			}
			auto member = std::make_shared<MemberExprNode>(prefix, std::move(memberID), std::move(castID));
			std::shared_ptr<Node> entity;
			if (propertyType == 0x0a || propertyType == 0x0c || propertyType == 0x15) {
				entity = readChunkRef(std::move(member));
			} else {
				entity = member;
			}
			return std::make_shared<ThePropExprNode>(std::move(entity), propName);
		}
		break;
	default:
		break;
	}
	return std::make_shared<CommentNode>("ERROR: Unknown property type " + std::to_string(propertyType));
}

std::shared_ptr<Node> Handler::readChunkRef(std::shared_ptr<Node> string) {
	auto lastLine = pop();
	auto firstLine = pop();
	auto lastItem = pop();
	auto firstItem = pop();
	auto lastWord = pop();
	auto firstWord = pop();
	auto lastChar = pop();
	auto firstChar = pop();

	if (!(firstLine->type == kLiteralNode && firstLine->getValue()->type == kDatumInt && firstLine->getValue()->toInt() == 0))
		string = std::make_shared<ChunkExprNode>(kChunkLine, std::move(firstLine), std::move(lastLine), std::move(string));
	if (!(firstItem->type == kLiteralNode && firstItem->getValue()->type == kDatumInt && firstItem->getValue()->toInt() == 0))
		string = std::make_shared<ChunkExprNode>(kChunkItem, std::move(firstItem), std::move(lastItem), std::move(string));
	if (!(firstWord->type == kLiteralNode && firstWord->getValue()->type == kDatumInt && firstWord->getValue()->toInt() == 0))
		string = std::make_shared<ChunkExprNode>(kChunkWord, std::move(firstWord), std::move(lastWord), std::move(string));
	if (!(firstChar->type == kLiteralNode && firstChar->getValue()->type == kDatumInt && firstChar->getValue()->toInt() == 0))
		string = std::make_shared<ChunkExprNode>(kChunkChar, std::move(firstChar), std::move(lastChar), std::move(string));

	return string;
}

void Handler::tagLoops() {
	// Tag any jmpifz which is a loop with the loop type
	// (kTagRepeatWhile, kTagRepeatWithIn, kTagRepeatWithTo, kTagRepeatWithDownTo).
	// Tag the instruction which `next repeat` jumps to with kTagNextRepeatTarget.
	// Tag any instructions which are internal loop logic with kTagSkip, so that
	// they will be skipped during translation.

	for (uint32_t startIndex = 0; startIndex < bytecodeArray.size(); startIndex++) {
		// All loops begin with jmpifz...
		auto &jmpifz = bytecodeArray[startIndex];
		if (jmpifz.opcode != kOpJmpIfZ)
			continue;

		// ...and end with endrepeat.
		uint32_t jmpPos = jmpifz.pos + jmpifz.obj;
		uint32_t endIndex = bytecodePosMap[jmpPos];
		auto &endRepeat = bytecodeArray[endIndex - 1];
		if (endRepeat.opcode != kOpEndRepeat || (endRepeat.pos - endRepeat.obj) > jmpifz.pos)
			continue;

		BytecodeTag loopType = identifyLoop(startIndex, endIndex);
		bytecodeArray[startIndex].tag = loopType;

		if (loopType == kTagRepeatWithIn) {
			for (uint32_t i = startIndex - 7, end = startIndex - 1; i <= end; i++)
				bytecodeArray[i].tag = kTagSkip;
			for (uint32_t i = startIndex + 1, end = startIndex + 5; i <= end; i++)
				bytecodeArray[i].tag = kTagSkip;
			bytecodeArray[endIndex - 3].tag = kTagNextRepeatTarget; // pushint8 1
			bytecodeArray[endIndex - 3].ownerLoop = startIndex;
			bytecodeArray[endIndex - 2].tag = kTagSkip; // add
			bytecodeArray[endIndex - 1].tag = kTagSkip; // endrepeat
			bytecodeArray[endIndex - 1].ownerLoop = startIndex;
			bytecodeArray[endIndex].tag = kTagSkip; // pop 3
		} else if (loopType == kTagRepeatWithTo || loopType == kTagRepeatWithDownTo) {
			uint32_t conditionStartIndex = bytecodePosMap[endRepeat.pos - endRepeat.obj];
			bytecodeArray[conditionStartIndex - 1].tag = kTagSkip; // set
			bytecodeArray[conditionStartIndex].tag = kTagSkip; // get
			bytecodeArray[startIndex - 1].tag = kTagSkip; // lteq / gteq
			bytecodeArray[endIndex - 5].tag = kTagNextRepeatTarget; // pushint8 1 / pushint8 -1
			bytecodeArray[endIndex - 5].ownerLoop = startIndex;
			bytecodeArray[endIndex - 4].tag = kTagSkip; // get
			bytecodeArray[endIndex - 3].tag = kTagSkip; // add
			bytecodeArray[endIndex - 2].tag = kTagSkip; // set
			bytecodeArray[endIndex - 1].tag = kTagSkip; // endrepeat
			bytecodeArray[endIndex - 1].ownerLoop = startIndex;
		} else if (loopType == kTagRepeatWhile) {
			bytecodeArray[endIndex - 1].tag = kTagNextRepeatTarget; // endrepeat
			bytecodeArray[endIndex - 1].ownerLoop = startIndex;
		}
	}
}

bool Handler::isRepeatWithIn(uint32_t startIndex, uint32_t endIndex) {
	if (startIndex < 7 || startIndex > bytecodeArray.size() - 6)
		return false;
	if (!(bytecodeArray[startIndex - 7].opcode == kOpPeek && bytecodeArray[startIndex - 7].obj == 0))
		return false;
	if (!(bytecodeArray[startIndex - 6].opcode == kOpPushArgList && bytecodeArray[startIndex - 6].obj == 1))
		return false;
	if (!(bytecodeArray[startIndex - 5].opcode == kOpExtCall && getName(bytecodeArray[startIndex - 5].obj) == "count"))
		return false;
	if (!(bytecodeArray[startIndex - 4].opcode == kOpPushInt8 && bytecodeArray[startIndex - 4].obj == 1))
		return false;
	if (!(bytecodeArray[startIndex - 3].opcode == kOpPeek && bytecodeArray[startIndex - 3].obj == 0))
		return false;
	if (!(bytecodeArray[startIndex - 2].opcode == kOpPeek && bytecodeArray[startIndex - 2].obj == 2))
		return false;
	if (!(bytecodeArray[startIndex - 1].opcode == kOpLtEq))
		return false;
	// if (!(bytecodeArray[startIndex].opcode == kOpJmpIfZ))
	//     return false;
	if (!(bytecodeArray[startIndex + 1].opcode == kOpPeek && bytecodeArray[startIndex + 1].obj == 2))
		return false;
	if (!(bytecodeArray[startIndex + 2].opcode == kOpPeek && bytecodeArray[startIndex + 2].obj == 1))
		return false;
	if (!(bytecodeArray[startIndex + 3].opcode == kOpPushArgList && bytecodeArray[startIndex + 3].obj == 2))
		return false;
	if (!(bytecodeArray[startIndex + 4].opcode == kOpExtCall && getName(bytecodeArray[startIndex + 4].obj) == "getAt"))
		return false;
	if (!(bytecodeArray[startIndex + 5].opcode == kOpSetGlobal || bytecodeArray[startIndex + 5].opcode == kOpSetProp
			|| bytecodeArray[startIndex + 5].opcode == kOpSetParam || bytecodeArray[startIndex + 5].opcode == kOpSetLocal))
		return false;

	if (endIndex < 3)
		return false;
	if (!(bytecodeArray[endIndex - 3].opcode == kOpPushInt8 && bytecodeArray[endIndex - 3].obj == 1))
		return false;
	if (!(bytecodeArray[endIndex - 2].opcode == kOpAdd))
		return false;
	// if (!(bytecodeArray[startIndex - 1].opcode == kOpEndRepeat))
	//     return false;
	if (!(bytecodeArray[endIndex].opcode == kOpPop && bytecodeArray[endIndex].obj == 3))
		return false;

	return true;
}

BytecodeTag Handler::identifyLoop(uint32_t startIndex, uint32_t endIndex) {
	if (isRepeatWithIn(startIndex, endIndex))
		return kTagRepeatWithIn;

	if (startIndex < 1)
		return kTagRepeatWhile;

	bool up;
	switch (bytecodeArray[startIndex - 1].opcode) {
	case kOpLtEq:
		up = true;
		break;
	case kOpGtEq:
		up = false;
		break;
	default:
		return kTagRepeatWhile;
	}

	auto &endRepeat = bytecodeArray[endIndex - 1];
	uint32_t conditionStartIndex = bytecodePosMap[endRepeat.pos - endRepeat.obj];

	if (conditionStartIndex < 1)
		return kTagRepeatWhile;

	OpCode getOp;
	switch (bytecodeArray[conditionStartIndex - 1].opcode) {
	case kOpSetGlobal:
		getOp = kOpGetGlobal;
		break;
	case kOpSetGlobal2:
		getOp = kOpGetGlobal2;
		break;
	case kOpSetProp:
		getOp = kOpGetProp;
		break;
	case kOpSetParam:
		getOp = kOpGetParam;
		break;
	case kOpSetLocal:
		getOp = kOpGetLocal;
		break;
	default:
		return kTagRepeatWhile;
	}
	OpCode setOp = bytecodeArray[conditionStartIndex - 1].opcode;
	int32_t varID = bytecodeArray[conditionStartIndex - 1].obj;

	if (!(bytecodeArray[conditionStartIndex].opcode == getOp && bytecodeArray[conditionStartIndex].obj == varID))
		return kTagRepeatWhile;

	if (endIndex < 5)
		return kTagRepeatWhile;
	if (up) {
		if (!(bytecodeArray[endIndex - 5].opcode == kOpPushInt8 && bytecodeArray[endIndex - 5].obj == 1))
			return kTagRepeatWhile;
	} else {
		if (!(bytecodeArray[endIndex - 5].opcode == kOpPushInt8 && bytecodeArray[endIndex - 5].obj == -1))
			return kTagRepeatWhile;
	}
	if (!(bytecodeArray[endIndex - 4].opcode == getOp && bytecodeArray[endIndex - 4].obj == varID))
		return kTagRepeatWhile;
	if (!(bytecodeArray[endIndex - 3].opcode == kOpAdd))
		return kTagRepeatWhile;
	if (!(bytecodeArray[endIndex - 2].opcode == setOp && bytecodeArray[endIndex - 2].obj == varID))
		return kTagRepeatWhile;

	return up ? kTagRepeatWithTo : kTagRepeatWithDownTo;
}

void Handler::translate() {
	tagLoops();
	stack.clear();
	ast = std::make_unique<AST>(this);
	uint32_t i = 0;
	while (i < bytecodeArray.size()) {
		auto &bytecode = bytecodeArray[i];
		uint32_t pos = bytecode.pos;
		// exit last block if at end
		while (pos == ast->currentBlock->endPos) {
			auto exitedBlock = ast->currentBlock;
			auto ancestorStmt = ast->currentBlock->ancestorStatement();
			ast->exitBlock();
			if (ancestorStmt) {
				if (ancestorStmt->type == kIfStmtNode) {
					auto ifStatement = static_cast<IfStmtNode *>(ancestorStmt);
					if (ifStatement->hasElse && exitedBlock == ifStatement->block1.get()) {
						ast->enterBlock(ifStatement->block2.get());
					}
				} else if (ancestorStmt->type == kCasesStmtNode) {
					auto casesStmt = static_cast<CasesStmtNode *>(ancestorStmt);
					auto caseNode = ast->currentBlock->currentCase;
					if (caseNode->expect == kCaseExpectOtherwise) {
						if (exitedBlock == caseNode->block.get()) {
							caseNode->otherwise = std::make_shared<BlockNode>();
							caseNode->otherwise->parent = caseNode;
							caseNode->otherwise->endPos = casesStmt->endPos;
							ast->enterBlock(caseNode->otherwise.get());
						} else {
							ast->currentBlock->currentCase = nullptr;
						}
					} else if (caseNode->expect == kCaseExpectPop) {
						ast->currentBlock->currentCase = nullptr;
					}
				}
			}
		}
		auto translateSize = translateBytecode(bytecode, i);
		i += translateSize;
	}
}

uint32_t Handler::translateBytecode(Bytecode &bytecode, uint32_t index) {
	if (bytecode.tag == kTagSkip || bytecode.tag == kTagNextRepeatTarget) {
		// This is internal loop logic. Skip it.
		return 1;
	}

	std::shared_ptr<Node> translation = nullptr;
	BlockNode *nextBlock = nullptr;

	switch (bytecode.opcode) {
	case kOpRet:
	case kOpRetFactory:
		if (index == bytecodeArray.size() - 1) {
			return 1; // end of handler
		}
		translation = std::make_shared<ExitStmtNode>();
		break;
	case kOpPushZero:
		translation = std::make_shared<LiteralNode>(std::make_shared<Datum>(0));
		break;
	case kOpMul:
	case kOpAdd:
	case kOpSub:
	case kOpDiv:
	case kOpMod:
	case kOpJoinStr:
	case kOpJoinPadStr:
	case kOpLt:
	case kOpLtEq:
	case kOpNtEq:
	case kOpEq:
	case kOpGt:
	case kOpGtEq:
	case kOpAnd:
	case kOpOr:
	case kOpContainsStr:
	case kOpContains0Str:
		{
			auto b = pop();
			auto a = pop();
			translation = std::make_shared<BinaryOpNode>(bytecode.opcode, std::move(a), std::move(b));
		}
		break;
	case kOpInv:
		{
			auto x = pop();
			translation = std::make_shared<InverseOpNode>(std::move(x));
		}
		break;
	case kOpNot:
		{
			auto x = pop();
			translation = std::make_shared<NotOpNode>(std::move(x));
		}
		break;
	case kOpGetChunk:
		{
			auto string = pop();
			translation = readChunkRef(std::move(string));
		}
		break;
	case kOpHiliteChunk:
		{
			std::shared_ptr<Node> castID;
			if (script->dir->version >= 500)
				castID = pop();
			auto fieldID = pop();
			auto field = std::make_shared<MemberExprNode>("field", std::move(fieldID), std::move(castID));
			auto chunk = readChunkRef(std::move(field));
			if (chunk->type == kCommentNode) { // error comment
				translation = chunk;
			} else {
				translation = std::make_shared<ChunkHiliteStmtNode>(std::move(chunk));
			}
		}
		break;
	case kOpOntoSpr:
		{
			auto secondSprite = pop();
			auto firstSprite = pop();
			translation = std::make_shared<SpriteIntersectsExprNode>(std::move(firstSprite), std::move(secondSprite));
		}
		break;
	case kOpIntoSpr:
		{
			auto secondSprite = pop();
			auto firstSprite = pop();
			translation = std::make_shared<SpriteWithinExprNode>(std::move(firstSprite), std::move(secondSprite));
		}
		break;
	case kOpGetField:
		{
			std::shared_ptr<Node> castID;
			if (script->dir->version >= 500)
				castID = pop();
			auto fieldID = pop();
			translation = std::make_shared<MemberExprNode>("field", std::move(fieldID), std::move(castID));
		}
		break;
	case kOpStartTell:
		{
			auto window = pop();
			auto tellStmt = std::make_shared<TellStmtNode>(std::move(window));
			translation = tellStmt;
			nextBlock = tellStmt->block.get();
		}
		break;
	case kOpEndTell:
		{
			ast->exitBlock();
			return 1;
		}
		break;
	case kOpPushList:
		{
			auto list = pop();
			list->getValue()->type = kDatumList;
			translation = list;
		}
		break;
	case kOpPushPropList:
		{
			auto list = pop();
			list->getValue()->type = kDatumPropList;
			translation = list;
		}
		break;
	case kOpSwap:
		if (stack.size() >= 2) {
			std::swap(stack[stack.size() - 1], stack[stack.size() - 2]);
		} else {
			Common::warning("kOpSwap: Stack too small!");
		}
		return 1;
	case kOpPushInt8:
	case kOpPushInt16:
	case kOpPushInt32:
		{
			auto i = std::make_shared<Datum>(bytecode.obj);
			translation = std::make_shared<LiteralNode>(std::move(i));
		}
		break;
	case kOpPushFloat32:
		{
			auto f = std::make_shared<Datum>(*(float *)(&bytecode.obj));
			translation = std::make_shared<LiteralNode>(std::move(f));
		}
		break;
	case kOpPushArgListNoRet:
		{
			auto argCount = bytecode.obj;
			std::vector<std::shared_ptr<Node>> args;
			while (argCount) {
				args.insert(args.begin(), pop());
				argCount--;
			}
			auto argList = std::make_shared<Datum>(kDatumArgListNoRet, args);
			translation = std::make_shared<LiteralNode>(std::move(argList));
		}
		break;
	case kOpPushArgList:
		{
			auto argCount = bytecode.obj;
			std::vector<std::shared_ptr<Node>> args;
			while (argCount) {
				args.insert(args.begin(), pop());
				argCount--;
			}
			auto argList = std::make_shared<Datum>(kDatumArgList, args);
			translation = std::make_shared<LiteralNode>(std::move(argList));
		}
		break;
	case kOpPushCons:
		{
			int literalID = bytecode.obj / variableMultiplier();
			if (-1 < literalID && (unsigned)literalID < script->literals.size()) {
				translation = std::make_shared<LiteralNode>(script->literals[literalID].value);
			} else {
				translation = std::make_shared<ErrorNode>();
			}
			break;
		}
	case kOpPushSymb:
		{
			auto sym = std::make_shared<Datum>(kDatumSymbol, getName(bytecode.obj));
			translation = std::make_shared<LiteralNode>(std::move(sym));
		}
		break;
	case kOpPushVarRef:
		{
			auto ref = std::make_shared<Datum>(kDatumVarRef, getName(bytecode.obj));
			translation = std::make_shared<LiteralNode>(std::move(ref));
		}
		break;
	case kOpGetGlobal:
	case kOpGetGlobal2:
		{
			auto name = getName(bytecode.obj);
			translation = std::make_shared<VarNode>(name);
		}
		break;
	case kOpGetProp:
		translation = std::make_shared<VarNode>(getName(bytecode.obj));
		break;
	case kOpGetParam:
		translation = std::make_shared<VarNode>(getArgumentName(bytecode.obj / variableMultiplier()));
		break;
	case kOpGetLocal:
		translation = std::make_shared<VarNode>(getLocalName(bytecode.obj / variableMultiplier()));
		break;
	case kOpSetGlobal:
	case kOpSetGlobal2:
		{
			auto varName = getName(bytecode.obj);
			auto var = std::make_shared<VarNode>(varName);
			auto value = pop();
			translation = std::make_shared<AssignmentStmtNode>(std::move(var), std::move(value));
		}
		break;
	case kOpSetProp:
		{
			auto var = std::make_shared<VarNode>(getName(bytecode.obj));
			auto value = pop();
			translation = std::make_shared<AssignmentStmtNode>(std::move(var), std::move(value));
		}
		break;
	case kOpSetParam:
		{
			auto var = std::make_shared<VarNode>(getArgumentName(bytecode.obj / variableMultiplier()));
			auto value = pop();
			translation = std::make_shared<AssignmentStmtNode>(std::move(var), std::move(value));
		}
		break;
	case kOpSetLocal:
		{
			auto var = std::make_shared<VarNode>(getLocalName(bytecode.obj / variableMultiplier()));
			auto value = pop();
			translation = std::make_shared<AssignmentStmtNode>(std::move(var), std::move(value));
		}
		break;
	case kOpJmp:
		{
			uint32_t targetPos = bytecode.pos + bytecode.obj;
			uint32_t targetIndex = bytecodePosMap[targetPos];
			auto ancestorLoop = ast->currentBlock->ancestorLoop();
			if (ancestorLoop) {
				if (bytecodeArray[targetIndex - 1].opcode == kOpEndRepeat && bytecodeArray[targetIndex - 1].ownerLoop == ancestorLoop->startIndex) {
					translation = std::make_shared<ExitRepeatStmtNode>();
					break;
				} else if (bytecodeArray[targetIndex].tag == kTagNextRepeatTarget && bytecodeArray[targetIndex].ownerLoop == ancestorLoop->startIndex) {
					translation = std::make_shared<NextRepeatStmtNode>();
					break;
				}
			}
			auto &nextBytecode = bytecodeArray[index + 1];
			auto ancestorStatement = ast->currentBlock->ancestorStatement();
			if (ancestorStatement && nextBytecode.pos == ast->currentBlock->endPos) {
				if (ancestorStatement->type == kIfStmtNode) {
					auto ifStmt = static_cast<IfStmtNode *>(ancestorStatement);
					if (ast->currentBlock == ifStmt->block1.get()) {
						ifStmt->hasElse = true;
						ifStmt->block2->endPos = targetPos;
						return 1; // if statement amended, nothing to push
					}
				} else if (ancestorStatement->type == kCasesStmtNode) {
					auto casesStmt = static_cast<CasesStmtNode *>(ancestorStatement);
					casesStmt->endPos = targetPos;
					return 1;
				}
			}
			translation = std::make_shared<CommentNode>("ERROR: Could not identify jmp");
		}
		break;
	case kOpEndRepeat:
		// This should normally be tagged kTagSkip or kTagNextRepeatTarget and skipped.
		translation = std::make_shared<CommentNode>("ERROR: Stray endrepeat");
		break;
	case kOpJmpIfZ:
		{
			uint32_t endPos = bytecode.pos + bytecode.obj;
			uint32_t endIndex = bytecodePosMap[endPos];
			switch (bytecode.tag) {
			case kTagRepeatWhile:
				{
					auto condition = pop();
					auto loop = std::make_shared<RepeatWhileStmtNode>(index, std::move(condition));
					loop->block->endPos = endPos;
					translation = loop;
					nextBlock = loop->block.get();
				}
				break;
			case kTagRepeatWithIn:
				{
					auto list = pop();
					std::string varName = getVarNameFromSet(bytecodeArray[index + 5]);
					auto loop = std::make_shared<RepeatWithInStmtNode>(index, varName, std::move(list));
					loop->block->endPos = endPos;
					translation = loop;
					nextBlock = loop->block.get();
				}
				break;
			case kTagRepeatWithTo:
			case kTagRepeatWithDownTo:
				{
					bool up = (bytecode.tag == kTagRepeatWithTo);
					auto end = pop();
					auto start = pop();
					auto endRepeat = bytecodeArray[endIndex - 1];
					uint32_t conditionStartIndex = bytecodePosMap[endRepeat.pos - endRepeat.obj];
					std::string varName = getVarNameFromSet(bytecodeArray[conditionStartIndex - 1]);
					auto loop = std::make_shared<RepeatWithToStmtNode>(index, varName, std::move(start), up, std::move(end));
					loop->block->endPos = endPos;
					translation = loop;
					nextBlock = loop->block.get();
				}
				break;
			default:
				{
					auto condition = pop();
					auto ifStmt = std::make_shared<IfStmtNode>(std::move(condition));
					ifStmt->block1->endPos = endPos;
					translation = ifStmt;
					nextBlock = ifStmt->block1.get();
				}
				break;
			}
		}
		break;
	case kOpLocalCall:
		{
			auto argList = pop();
			translation = std::make_shared<CallNode>(script->handlers[bytecode.obj]->name, std::move(argList));
		}
		break;
	case kOpExtCall:
	case kOpTellCall:
		{
			auto argList = pop();
			translation = std::make_shared<CallNode>(getName(bytecode.obj), std::move(argList));
		}
		break;
	case kOpObjCallV4:
		{
			auto object = readVar(bytecode.obj);
			auto argList = pop();
			auto &rawArgList = argList->getValue()->l;
			if (rawArgList.size() > 0) {
				// first arg is a symbol
				// replace it with a variable
				rawArgList[0] = std::make_shared<VarNode>(rawArgList[0]->getValue()->s);
			}
			translation = std::make_shared<ObjCallV4Node>(std::move(object), std::move(argList));
		}
		break;
	case kOpPut:
		{
			PutType putType = static_cast<PutType>((bytecode.obj >> 4) & 0xF);
			uint32_t varType = bytecode.obj & 0xF;
			auto var = readVar(varType);
			auto val = pop();
			translation = std::make_shared<PutStmtNode>(putType, std::move(var), std::move(val));
		}
		break;
	case kOpPutChunk:
		{
			PutType putType = static_cast<PutType>((bytecode.obj >> 4) & 0xF);
			uint32_t varType = bytecode.obj & 0xF;
			auto var = readVar(varType);
			auto chunk = readChunkRef(std::move(var));
			auto val = pop();
			if (chunk->type == kCommentNode) { // error comment
				translation = chunk;
			} else {
				translation = std::make_shared<PutStmtNode>(putType, std::move(chunk), std::move(val));
			}
		}
		break;
	case kOpDeleteChunk:
		{
			auto var = readVar(bytecode.obj);
			auto chunk = readChunkRef(std::move(var));
			if (chunk->type == kCommentNode) { // error comment
				translation = chunk;
			} else {
				translation = std::make_shared<ChunkDeleteStmtNode>(std::move(chunk));
			}
		}
		break;
	case kOpGet:
		{
			int propertyID = pop()->getValue()->toInt();
			translation = readV4Property(bytecode.obj, propertyID);
		}
		break;
	case kOpSet:
		{
			int propertyID = pop()->getValue()->toInt();
			auto value = pop();
			if (bytecode.obj == 0x00 && 0x01 <= propertyID && propertyID <= 0x05 && value->getValue()->type == kDatumString) {
				// This is either a `set eventScript to "script"` or `when event then script` statement.
				// If the script starts with a space, it's probably a when statement.
				// If the script contains a line break, it's definitely a when statement.
				std::string script = value->getValue()->s;
				if (script.size() > 0 && (script[0] == ' ' || script.find(kLingoLineEnding) != std::string::npos)) {
					translation = std::make_shared<WhenStmtNode>(propertyID, script);
				}
			}
			if (!translation) {
				auto prop = readV4Property(bytecode.obj, propertyID);
				if (prop->type == kCommentNode) { // error comment
					translation = prop;
				} else {
					translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value), true);
				}
			}
		}
		break;
	case kOpGetMovieProp:
		translation = std::make_shared<TheExprNode>(getName(bytecode.obj));
		break;
	case kOpSetMovieProp:
		{
			auto value = pop();
			auto prop = std::make_shared<TheExprNode>(getName(bytecode.obj));
			translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
		}
		break;
	case kOpGetObjProp:
	case kOpGetChainedProp:
		{
			auto object = pop();
			translation = std::make_shared<ObjPropExprNode>(std::move(object), getName(bytecode.obj));
		}
		break;
	case kOpSetObjProp:
		{
			auto value = pop();
			auto object = pop();
			auto prop = std::make_shared<ObjPropExprNode>(std::move(object), getName(bytecode.obj));
			translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
		}
		break;
	case kOpPeek:
		{
			// This op denotes the beginning of a 'repeat with ... in list' statement or a case in a cases statement.

			// In a 'repeat with ... in list' statement, this peeked value is the list.
			// In a cases statement, this is the switch expression.
			auto peekedValue = peek();

			auto prevCase = ast->currentBlock->currentCase;

			// This must be a case. Find the comparison against the switch expression.
			auto originalStackSize = stack.size();
			uint32_t currIndex = index + 1;
			Bytecode *currBytecode = &bytecodeArray[currIndex];
			do {
				translateBytecode(*currBytecode, currIndex);
				currIndex += 1;
				currBytecode = &bytecodeArray[currIndex];
			} while (
				currIndex < bytecodeArray.size()
				&& !(stack.size() == originalStackSize + 1 && (currBytecode->opcode == kOpEq || currBytecode->opcode == kOpNtEq))
			);
			if (currIndex >= bytecodeArray.size()) {
				bytecode.translation = std::make_shared<CommentNode>("ERROR: Expected eq or nteq!");
				ast->addStatement(bytecode.translation);
				return currIndex - index + 1;
			}

			// If the comparison is <>, this is followed by another, equivalent case.
			// (e.g. this could be case1 in `case1, case2: statement`)
			bool notEq = (currBytecode->opcode == kOpNtEq);
			std::shared_ptr<Node> caseValue = pop(); // This is the value the switch expression is compared against.

			currIndex += 1;
			currBytecode = &bytecodeArray[currIndex];
			if (currIndex >= bytecodeArray.size() || currBytecode->opcode != kOpJmpIfZ) {
				bytecode.translation = std::make_shared<CommentNode>("ERROR: Expected jmpifz!");
				ast->addStatement(bytecode.translation);
				return currIndex - index + 1;
			}

			auto &jmpifz = *currBytecode;
			auto jmpPos = jmpifz.pos + jmpifz.obj;
			auto &targetBytecode = bytecodeArray[bytecodePosMap[jmpPos]];
			CaseExpect expect;
			if (notEq)
				expect = kCaseExpectOr; // Expect an equivalent case after this one.
			else if (targetBytecode.opcode == kOpPeek)
				expect = kCaseExpectNext; // Expect a different case after this one.
			else if (targetBytecode.opcode == kOpPop)
				expect = kCaseExpectPop; // Expect the end of the switch statement (where the switch expression is popped off the stack).
			else
				expect = kCaseExpectOtherwise; // Expect an 'otherwise' block.

			auto currCase = std::make_shared<CaseNode>(std::move(caseValue), expect);
			jmpifz.translation = currCase;
			ast->currentBlock->currentCase = currCase.get();

			if (!prevCase) {
				auto casesStmt = std::make_shared<CasesStmtNode>(std::move(peekedValue));
				casesStmt->firstCase = currCase;
				currCase->parent = casesStmt.get();
				bytecode.translation = casesStmt;
				ast->addStatement(casesStmt);
			} else if (prevCase->expect == kCaseExpectOr) {
				prevCase->nextOr = currCase;
				currCase->parent = prevCase;
			} else if (prevCase->expect == kCaseExpectNext) {
				prevCase->nextCase = currCase;
				currCase->parent = prevCase;
			}

			// The block doesn't start until the after last equivalent case,
			// so don't create a block yet if we're expecting an equivalent case.
			if (currCase->expect != kCaseExpectOr) {
				currCase->block = std::make_shared<BlockNode>();
				currCase->block->parent = currCase.get();
				currCase->block->endPos = jmpPos;
				ast->enterBlock(currCase->block.get());
			}

			return currIndex - index + 1;
		}
		break;
	case kOpPop:
		for (int i = 0; i < bytecode.obj; i++) {
			pop();
		}
		return 1;
	case kOpTheBuiltin:
		{
			pop(); // empty arglist
			translation = std::make_shared<TheExprNode>(getName(bytecode.obj));
		}
		break;
	case kOpObjCall:
		{
			std::string method = getName(bytecode.obj);
			auto argList = pop();
			auto &rawArgList = argList->getValue()->l;
			size_t nargs = rawArgList.size();
			if (method == "getAt" && nargs == 2)  {
				// obj.getAt(i) => obj[i]
				auto obj = rawArgList[0];
				auto prop = rawArgList[1];
				translation = std::make_shared<ObjBracketExprNode>(std::move(obj), std::move(prop));
			} else if (method == "setAt" && nargs == 3) {
				// obj.setAt(i) => obj[i] = val
				auto obj = rawArgList[0];
				auto prop = rawArgList[1];
				auto val = rawArgList[2];
				std::shared_ptr<Node> propExpr = std::make_shared<ObjBracketExprNode>(std::move(obj), std::move(prop));
				translation = std::make_shared<AssignmentStmtNode>(std::move(propExpr), std::move(val));
			} else if ((method == "getProp" || method == "getPropRef") && (nargs == 3 || nargs == 4) && rawArgList[1]->getValue()->type == kDatumSymbol) {
				// obj.getProp(#prop, i) => obj.prop[i]
				// obj.getProp(#prop, i, i2) => obj.prop[i..i2]
				auto obj = rawArgList[0];
				std::string propName  = rawArgList[1]->getValue()->s;
				auto i = rawArgList[2];
				auto i2 = (nargs == 4) ? rawArgList[3] : nullptr;
				translation = std::make_shared<ObjPropIndexExprNode>(std::move(obj), propName, std::move(i), std::move(i2));
			} else if (method == "setProp" && (nargs == 4 || nargs == 5) && rawArgList[1]->getValue()->type == kDatumSymbol) {
				// obj.setProp(#prop, i, val) => obj.prop[i] = val
				// obj.setProp(#prop, i, i2, val) => obj.prop[i..i2] = val
				auto obj = rawArgList[0];
				std::string propName  = rawArgList[1]->getValue()->s;
				auto i = rawArgList[2];
				auto i2 = (nargs == 5) ? rawArgList[3] : nullptr;
				auto propExpr = std::make_shared<ObjPropIndexExprNode>(std::move(obj), propName, std::move(i), std::move(i2));
				auto val = rawArgList[nargs - 1];
				translation = std::make_shared<AssignmentStmtNode>(std::move(propExpr), std::move(val));
			} else if (method == "count" && nargs == 2 && rawArgList[1]->getValue()->type == kDatumSymbol) {
				// obj.count(#prop) => obj.prop.count
				auto obj = rawArgList[0];
				std::string propName  = rawArgList[1]->getValue()->s;
				auto propExpr = std::make_shared<ObjPropExprNode>(std::move(obj), propName);
				translation = std::make_shared<ObjPropExprNode>(std::move(propExpr), "count");
			} else if ((method == "setContents" || method == "setContentsAfter" || method == "setContentsBefore") && nargs == 2) {
				// var.setContents(val) => put val into var
				// var.setContentsAfter(val) => put val after var
				// var.setContentsBefore(val) => put val before var
				PutType putType;
				if (method == "setContents") {
					putType = kPutInto;
				} else if (method == "setContentsAfter") {
					putType = kPutAfter;
				} else {
					putType = kPutBefore;
				}
				auto var = rawArgList[0];
				auto val = rawArgList[1];
				translation = std::make_shared<PutStmtNode>(putType, std::move(var), std::move(val));
			} else if (method == "hilite" && nargs == 1) {
				// chunk.hilite() => hilite chunk
				auto chunk = rawArgList[0];
				translation = std::make_shared<ChunkHiliteStmtNode>(chunk);
			} else if (method == "delete" && nargs == 1) {
				// chunk.delete() => delete chunk
				auto chunk = rawArgList[0];
				translation = std::make_shared<ChunkDeleteStmtNode>(chunk);
			} else {
				translation = std::make_shared<ObjCallNode>(method, std::move(argList));
			}
		}
		break;
	case kOpPushChunkVarRef:
		translation = readVar(bytecode.obj);
		break;
	case kOpGetTopLevelProp:
		{
			auto name = getName(bytecode.obj);
			translation = std::make_shared<VarNode>(name);
		}
		break;
	case kOpNewObj:
		{
			auto objType = getName(bytecode.obj);
			auto objArgs = pop();
			translation = std::make_shared<NewObjNode>(objType, std::move(objArgs));
		}
		break;
	default:
		{
			auto commentText = Lingo::getOpcodeName(bytecode.opID);
			if (bytecode.opcode >= 0x40)
				commentText += " " + std::to_string(bytecode.obj);
			translation = std::make_shared<CommentNode>(commentText);
			stack.clear(); // Clear stack so later bytecode won't be too screwed up
		}
	}

	if (!translation)
		translation = std::make_shared<ErrorNode>();

	bytecode.translation = translation;
	if (translation->isExpression) {
		stack.push_back(std::move(translation));
	} else {
		ast->addStatement(std::move(translation));
	}

	if (nextBlock)
		ast->enterBlock(nextBlock);

	return 1;
}

std::string posToString(int32_t pos) {
	std::stringstream ss;
	ss << "[" << std::setfill(' ') << std::setw(3) << pos << "]";
	return ss.str();
}

std::string Handler::bytecodeText() {
	bool dotSyntax = script->dir->dotSyntax;

	std::string res = "on " + name;
	if (argumentNames.size() > 0) {
		res += " ";
		for (size_t i = 0; i < argumentNames.size(); i++) {
			if (i > 0)
				res += ", ";
			res += argumentNames[i];
		}
	}
	res += kLingoLineEnding;
	for (auto &bytecode : bytecodeArray) {
		auto line = "  " + posToString(bytecode.pos) + " " + Lingo::getOpcodeName(bytecode.opID);
		switch (bytecode.opcode) {
		case kOpJmp:
		case kOpJmpIfZ:
			line += " " + posToString(bytecode.pos + bytecode.obj);
			break;
		case kOpEndRepeat:
			line += " " + posToString(bytecode.pos - bytecode.obj);
			break;
		case kOpPushFloat32:
			line += " " + floatToString(*(float *)(&bytecode.obj));
			break;
		default:
			if (bytecode.opID > 0x40)
				line += " " + std::to_string(bytecode.obj);
			break;
		}
		if (bytecode.translation) {
			line += " ...";
			while (line.size() < 49) {
				line += ".";
			}
			line += " ";
			if (bytecode.translation->isExpression)
				line += "<" + bytecode.translation->toString(dotSyntax, true) + ">";
			else
				line += bytecode.translation->toString(dotSyntax, true);
		}
		line += kLingoLineEnding;
		res += line;
	}
	res += "end";
	res += kLingoLineEnding;
	return res;
}

} // namespace Director
