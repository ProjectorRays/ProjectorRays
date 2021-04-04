#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <boost/format.hpp>

#include "chunk.h"
#include "lingo.h"
#include "movie.h"
#include "stream.h"

namespace ProjectorRays {

/* Handler */

void Handler::readRecord(ReadStream &stream) {
    nameID = stream.readInt16();
    vectorPos = stream.readUint16();
    compiledLen = stream.readUint32();
    compiledOffset = stream.readUint32();
    argumentCount = stream.readUint16();
    argumentOffset = stream.readUint32();
    localsCount = stream.readUint16();
    localsOffset = stream.readUint32();
    unknown0Count = stream.readUint16();
    unknown0Offset = stream.readUint32();
    unknown1 = stream.readUint32();
    unknown2 = stream.readUint16();
    lineCount = stream.readUint16();
    lineOffset = stream.readUint32();
    // yet to implement
    if (script->movie->capitalX)
        stackHeight = stream.readUint32();
}

void Handler::readData(ReadStream &stream) {
    stream.seek(compiledOffset);
    while (stream.pos() < compiledOffset + compiledLen) {
        auto pos = stream.pos();
        auto op = stream.readUint8();
        // instructions can be one, two or three bytes
        auto obj = 0;
        if (op >= 0xc0) {
            obj = stream.readInt32();
        } else if (op >= 0x80) {
            obj = stream.readUint16();
        } else if (op >= 0x40) {
            obj = stream.readUint8();
        }
        // read the first byte to convert to an opcode
        Bytecode bytecode(op, obj, pos - compiledOffset);
        bytecodeArray.push_back(bytecode);
        bytecodePosMap[pos - compiledOffset] = bytecodeArray.size() - 1;
    }

    argumentNameIDs = readVarnamesTable(stream, argumentCount, argumentOffset);
    localNameIDs = readVarnamesTable(stream, localsCount, localsOffset);
}

std::vector<int16_t> Handler::readVarnamesTable(ReadStream &stream, uint16_t count, uint32_t offset) {
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
    // For now approximating it with the point at which Ltcx changed to LctX.
    if (script->movie->capitalX)
        return 1;
    if (script->movie->version >= 500)
        return 8;
    return 6;
}

void Handler::registerGlobal(const std::string &name) {
    if (std::find(script->globalNames.begin(), script->globalNames.end(), name) == script->globalNames.end()
            && std::find(globalNames.begin(), globalNames.end(), name) == globalNames.end()) {
        globalNames.push_back(name);
    }
}

std::shared_ptr<Node> Handler::findVar(int varType, std::shared_ptr<Datum> id) {
	switch (varType) {
	case 0x1: // global
	case 0x2: // global
	case 0x3: // property/instance
		return std::make_shared<LiteralNode>(std::move(id));
	case 0x4: // arg
        {
            std::string name = getArgumentName(id->i / variableMultiplier());
            auto ref = std::make_shared<Datum>(kDatumVarRef, name);
            return std::make_shared<LiteralNode>(std::move(ref));
        }
	case 0x5: // local
        {
            std::string name = getLocalName(id->i / variableMultiplier());
            auto ref = std::make_shared<Datum>(kDatumVarRef, name);
            return std::make_shared<LiteralNode>(std::move(ref));
        }
	case 0x6: // field
        {
            auto fieldName = std::make_shared<LiteralNode>(id);
            return std::make_shared<FieldExprNode>(std::move(fieldName));
        }
	default:
		std::cout << boost::format("findVar: unhandled var type %d\n") % varType;
		break;
	}
	return std::make_shared<ErrorNode>();
}

std::shared_ptr<RepeatWithInStmtNode> Handler::buildRepeatWithIn(size_t index) {
    if (index >= bytecodeArray.size() - 12)
        return nullptr;
    if (!(bytecodeArray[index + 1].opcode == kOpPushArgList && bytecodeArray[index + 1].obj == 1))
        return nullptr;
    if (!(bytecodeArray[index + 2].opcode == kOpCallExt && getName(bytecodeArray[index + 2].obj) == "count"))
        return nullptr;
    if (!(bytecodeArray[index + 3].opcode == kOpPushInt41 && bytecodeArray[index + 3].obj == 1))
        return nullptr;
    if (!(bytecodeArray[index + 4].opcode == kOpPeek && bytecodeArray[index + 4].obj == 0))
        return nullptr;
    if (!(bytecodeArray[index + 5].opcode == kOpPeek && bytecodeArray[index + 5].obj == 2))
        return nullptr;
    if (!(bytecodeArray[index + 6].opcode == kOpLtEq))
        return nullptr;
    if (!(bytecodeArray[index + 7].opcode == kOpJmpIfZ))
        return nullptr;

    size_t endPos = bytecodeArray[index + 7].pos + 8 + bytecodeArray[index + 7].obj;

    if (!(bytecodeArray[index + 8].opcode == kOpPeek && bytecodeArray[index + 8].obj == 2))
        return nullptr;
    if (!(bytecodeArray[index + 9].opcode == kOpPeek && bytecodeArray[index + 9].obj == 1))
        return nullptr;
    if (!(bytecodeArray[index + 10].opcode == kOpPushArgList && bytecodeArray[index + 10].obj == 2))
        return nullptr;
    if (!(bytecodeArray[index + 11].opcode == kOpCallExt && getName(bytecodeArray[index + 11].obj) == "getAt"))
        return nullptr;

    std::string varName;
    switch (bytecodeArray[index + 12].opcode) {
        case kOpSetGlobal:
            varName = getName(bytecodeArray[index + 12].obj);
            registerGlobal(varName);
            break;
        case kOpSetProp:
            varName = getName(bytecodeArray[index + 12].obj);
            break;
        case kOpSetParam:
            varName = getArgumentName(bytecodeArray[index + 12].obj / variableMultiplier());
            break;
        case kOpSetLocal:
            varName = getLocalName(bytecodeArray[index + 12].obj / variableMultiplier());
            break;
        default:
            return nullptr;
    }

    auto res = std::make_shared<RepeatWithInStmtNode>(varName);
    res->block->endPos = endPos;
    return res;
}

void Handler::translate() {
    stack.clear();
    ast = std::make_unique<AST>(this);
    size_t i = 0;
    while (i < bytecodeArray.size()) {
        auto &bytecode = bytecodeArray[i];
        auto pos = bytecode.pos;
        // exit last block if at end
        while (pos == ast->currentBlock->endPos) {
            auto exitedBlock = ast->currentBlock;
            auto ancestorStmt = ast->currentBlock->ancestorStatement();
            ast->exitBlock();
            if (ancestorStmt) {
                if (ancestorStmt->type == kIfStmtNode) {
                    auto ifStatement = static_cast<IfStmtNode *>(ancestorStmt);
                    if (ifStatement->ifType == kIfElse && exitedBlock == ifStatement->block1.get()) {
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

size_t Handler::translateBytecode(Bytecode &bytecode, size_t index) {
    std::shared_ptr<Node> comment = nullptr;
    std::shared_ptr<Node> translation = nullptr;
    BlockNode *nextBlock = nullptr;

    switch (bytecode.opcode) {
    case kOpRet:
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
            auto lastLine = pop();
            auto firstLine = pop();
            auto lastItem = pop();
            auto firstItem = pop();
            auto lastWord = pop();
            auto firstWord = pop();
            auto lastChar = pop();
            auto firstChar = pop();
            if (firstChar->getValue()->toInt()) {
                translation = std::make_shared<ChunkExprNode>(kChunkChar, std::move(firstChar), std::move(lastChar), std::move(string));
            } else if (firstWord->getValue()->toInt()) {
                translation = std::make_shared<ChunkExprNode>(kChunkWord, std::move(firstWord), std::move(lastWord), std::move(string));
            } else if (firstItem->getValue()->toInt()) {
                translation = std::make_shared<ChunkExprNode>(kChunkItem, std::move(firstItem), std::move(lastItem), std::move(string));
            } else if (firstLine->getValue()->toInt()) {
                translation = std::make_shared<ChunkExprNode>(kChunkLine, std::move(firstLine), std::move(lastLine), std::move(string));
            } else {
                translation = std::make_shared<ErrorNode>();
            }
        }
        break;
    case kOpHiliteChunk:
        {
            if (script->movie->version >= 500)
                auto castID = pop();
            auto fieldID = pop();
            auto lastLine = pop();
            auto firstLine = pop();
            auto lastItem = pop();
            auto firstItem = pop();
            auto lastWord = pop();
            auto firstWord = pop();
            auto lastChar = pop();
            auto firstChar = pop();
            if (firstChar->getValue()->toInt()) {
                translation = std::make_shared<ChunkHiliteStmtNode>(kChunkChar, std::move(firstChar), std::move(lastChar), std::move(fieldID));
            } else if (firstWord->getValue()->toInt()) {
                translation = std::make_shared<ChunkHiliteStmtNode>(kChunkWord, std::move(firstWord), std::move(lastWord), std::move(fieldID));
            } else if (firstItem->getValue()->toInt()) {
                translation = std::make_shared<ChunkHiliteStmtNode>(kChunkItem, std::move(firstItem), std::move(lastItem), std::move(fieldID));
            } else if (firstLine->getValue()->toInt()) {
                translation = std::make_shared<ChunkHiliteStmtNode>(kChunkLine, std::move(firstLine), std::move(lastLine), std::move(fieldID));
            } else {
                translation = std::make_shared<ErrorNode>();
            }
        }
        break;
    case kOpOntoSpr:
        {
            auto firstSprite = pop();
            auto secondSprite = pop();
            translation = std::make_shared<SpriteIntersectsExprNode>(std::move(firstSprite), std::move(secondSprite));
        }
        break;
    case kOpIntoSpr:
        {
            auto firstSprite = pop();
            auto secondSprite = pop();
            translation = std::make_shared<SpriteWithinExprNode>(std::move(firstSprite), std::move(secondSprite));
        }
        break;
    case kOpGetField:
        {
            if (script->movie->version >= 500)
                auto castID = pop();
            auto fieldID = pop();
            translation = std::make_shared<FieldExprNode>(std::move(fieldID));
        }
        break;
    case kOpStartObj:
        pop();
        // TODO
        break;
    case kOpStopObj:
        // TODO
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
    case kOpPushInt41:
    case kOpPushInt6E:
        {
            auto i = std::make_shared<Datum>((int)bytecode.obj);
            translation = std::make_shared<LiteralNode>(std::move(i));
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
        {
            auto name = getName(bytecode.obj);
            registerGlobal(name);
            translation = std::make_shared<VarNode>(name);
        }
        break;
    case kOpGetProp:
        translation = std::make_shared<VarNode>(getName(bytecode.obj));
        break;
    case kOpGetParam:
        translation = std::make_shared<VarNode>(getName(bytecode.obj / variableMultiplier()));
        break;
    case kOpGetLocal:
        translation = std::make_shared<VarNode>(getLocalName(bytecode.obj / variableMultiplier()));
        break;
    case kOpSetGlobal:
        {
            auto varName = getName(bytecode.obj);
            registerGlobal(varName);
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
            auto targetPos = bytecode.pos + bytecode.obj;
            auto &nextBytecode = bytecodeArray[index + 1];
            auto &targetBytecode = bytecodeArray[bytecodePosMap[targetPos]];
            auto &targetPrevBytecode = bytecodeArray[index - 1];
            auto ancestorStatement = ast->currentBlock->ancestorStatement();

            if (ancestorStatement) {
                if (ancestorStatement->type == kIfStmtNode) {
                    if (nextBytecode.pos == ast->currentBlock->endPos && targetPrevBytecode.opcode == kOpEndRepeat) {
                        translation = std::make_shared<ExitRepeatStmtNode>();
                    } else if (targetBytecode.opcode == kOpEndRepeat) {
                        translation = std::make_shared<NextRepeatStmtNode>();
                    } else if (nextBytecode.pos == ast->currentBlock->endPos) {
                        auto ifStmt = static_cast<IfStmtNode *>(ancestorStatement);
                        ifStmt->ifType = kIfElse;
                        ifStmt->block2->endPos = targetPos;
                        return 1; // if statement amended, nothing to push
                    }
                } else if (ancestorStatement->type == kCasesStmtNode) {
                    auto casesStmt = static_cast<CasesStmtNode *>(ancestorStatement);
                    casesStmt->endPos = targetPos;
                    return 1;
                }
            }
        }
        break;
    case kOpEndRepeat:
        {
            auto targetPos = bytecode.pos - bytecode.obj;
            auto i = bytecodePosMap[targetPos];
            while (bytecodeArray[i].opcode != kOpJmpIfZ) i++; // TODO: See if this can be removed
            auto &targetBytecode = bytecodeArray[i];
            if (targetBytecode.translation && targetBytecode.translation->type == kIfStmtNode) {
                auto ifStmt = std::static_pointer_cast<IfStmtNode>(targetBytecode.translation);
                ifStmt->ifType = kRepeatWhile;
            }
            return 1; // if statement amended, nothing to push
        }
        break;
    case kOpJmpIfZ:
        {
            auto endPos = bytecode.pos + bytecode.obj;
            auto condition = pop();
            auto ifStmt = std::make_shared<IfStmtNode>(std::move(condition));
            ifStmt->block1->endPos = endPos;
            translation = ifStmt;
            nextBlock = ifStmt->block1.get();
        }
        break;
    case kOpCallLocal:
        {
            auto argList = pop();
            translation = std::make_shared<CallNode>(script->handlers[bytecode.obj]->name, std::move(argList));
        }
        break;
    case kOpCallExt:
        {
            auto argList = pop();
            translation = std::make_shared<CallNode>(getName(bytecode.obj), std::move(argList));
        }
        break;
    case kOpCallObjOld:
        // Possibly used by old Director versions?
        /* auto object = */ pop();
        /* let argList = */ pop();
        // TODO
        break;
    case kOpPut:
        {
            PutType putType = static_cast<PutType>((bytecode.obj >> 4) & 0xF);
            uint32_t varType = bytecode.obj & 0xF;
            if (varType == 6 && script->movie->version >= 500)
                auto castID = pop(); // field cast ID

            auto varId = pop()->getValue();
            auto var = findVar(varType, std::move(varId));
            auto val = pop();
            translation = std::make_shared<PutStmtNode>(putType, var, val);
        }
        break;
    case kOp5BXX:
        pop();
        // TODO
        break;
    case kOpGet:
        switch (bytecode.obj) {
        case 0x00:
            {
                auto id = pop()->getValue()->toInt();
                if (id <= 0x0b) {
                    auto propName = Lingo::getName(Lingo::moviePropertyNames00, id);
                    translation = std::make_shared<TheExprNode>(propName);
                } else {
                    auto string = pop();
                    auto chunkType = static_cast<ChunkType>(id - 0x0b);
                    translation = std::make_shared<LastStringChunkExprNode>(chunkType, std::move(string));
                }
            }
            break;
        case 0x01:
            {
                auto chunkType = static_cast<ChunkType>(pop()->getValue()->toInt());
                auto string = pop();
                translation = std::make_shared<StringChunkCountExprNode>(chunkType, std::move(string));
            }
            break;
        case 0x02:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto menuID = pop();
                translation = std::make_shared<MenuPropExprNode>(std::move(menuID), propertyID);
            }
            break;
        case 0x03:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto menuID = pop();
                auto itemID = pop();
                translation = std::make_shared<MenuItemPropExprNode>(std::move(menuID), std::move(itemID), propertyID);
            }
            break;
        case 0x04:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto soundID = pop();
                translation = std::make_shared<SoundPropExprNode>(std::move(soundID), propertyID);
            }
            break;
        case 0x06:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto spriteID = pop();
                translation = std::make_shared<SoundPropExprNode>(std::move(spriteID), propertyID);
            }
            break;
        case 0x07:
            {
                auto propertyID = pop()->getValue()->toInt();
                translation = std::make_shared<TheExprNode>(Lingo::getName(Lingo::moviePropertyNames07, propertyID));
            }
            break;
        case 0x08:
            {
                auto propertyID = pop()->getValue()->toInt();
                translation = std::make_shared<TheExprNode>(Lingo::getName(Lingo::moviePropertyNames08, propertyID));
            }
            break;
        case 0x09:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto propName = Lingo::getName(Lingo::castPropertyNames09, propertyID);
                auto castID = pop();
                translation = std::make_shared<CastPropExprNode>(std::move(castID), propName);
            }
            break;
        case 0x0c:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto fieldID = pop();
                translation = std::make_shared<FieldPropExprNode>(std::move(fieldID), propertyID);
            }
            break;
        case 0x0d:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto propName = Lingo::getName(Lingo::castPropertyNames0D, propertyID);
                auto castID = pop();
                translation = std::make_shared<CastPropExprNode>(std::move(castID), propName);
            }
            break;
        default:
            translation = std::make_shared<ErrorNode>();
        }
        break;
    case kOpSet:
        switch (bytecode.obj) {
        case 0x00:
            {
                auto id = pop()->getValue()->toInt();
                auto value = pop();
                if (id <= 0x05) {
                    auto propName = Lingo::getName(Lingo::moviePropertyNames00, id);
                    std::shared_ptr<Node> prop = std::make_shared<TheExprNode>(propName);
                    translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
                } else {
                    translation = std::make_shared<ErrorNode>();
                }
            }
            break;
        case 0x03:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto menuID = pop();
                auto itemID = pop();
                auto prop = std::make_shared<MenuItemPropExprNode>(std::move(menuID), std::move(itemID), propertyID);
                translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x04:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto soundID = pop();
                auto prop = std::make_shared<SoundPropExprNode>(std::move(soundID), propertyID);
                translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x06:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto spriteID = pop();
                auto prop = std::make_shared<SoundPropExprNode>(std::move(spriteID), propertyID);
                translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x07:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto prop = std::make_shared<TheExprNode>(Lingo::getName(Lingo::moviePropertyNames07, propertyID));
                translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x09:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto propName = Lingo::getName(Lingo::castPropertyNames09, propertyID);
                auto castID = pop();
                auto prop = std::make_shared<CastPropExprNode>(std::move(castID), propName);
                translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x0c:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto fieldID = pop();
                auto prop = std::make_shared<FieldPropExprNode>(std::move(fieldID), propertyID);
                translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x0d:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto propName = Lingo::getName(Lingo::castPropertyNames0D, propertyID);
                auto castID = pop();
                auto prop = std::make_shared<CastPropExprNode>(std::move(castID), propName);
                translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        default:
            translation = std::make_shared<ErrorNode>();
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
            if (!prevCase) {
                // Try to a build a 'repeat with ... in list' statement with the following bytecode.
                auto repeatWithIn = buildRepeatWithIn(index);
                if (repeatWithIn) {
                    stack.push_back(std::make_shared<TempNode>());
                    stack.push_back(std::make_shared<TempNode>());
                    repeatWithIn->list = std::move(peekedValue);
                    bytecode.translation = repeatWithIn;
                    ast->addStatement(repeatWithIn);
                    ast->enterBlock(repeatWithIn->block.get());
                    return 13;
                }
            }

            // This must be a case. Find the comparison against the switch expression.
            auto originalStackSize = stack.size();
            size_t currIndex = index + 1;
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
                throw new std::runtime_error("kOpPeek: Out of bounds!");
            }

            // If the comparison is <>, this is followed by another, equivalent case.
            // (e.g. this could be case1 in `case1, case2: statement`)
            bool notEq = (currBytecode->opcode == kOpNtEq);
            std::shared_ptr<Node> caseValue = pop(); // This is the value the switch expression is compared against.

            currIndex += 1;
            if (currIndex >= bytecodeArray.size()) {
                throw new std::runtime_error("kOpPeek: Out of bounds!");
            }
            currBytecode = &bytecodeArray[currIndex];
            if (currBytecode->opcode != kOpJmpIfZ) {
                throw new std::runtime_error(boost::str(
                    boost::format("Expected jmpifz at index %zu") % currIndex
                ));
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
    case kOpGetMovieInfo:
        {
            pop(); // FIXME: What is this?
            translation = std::make_shared<TheExprNode>(getName(bytecode.obj));
        }
        break;
    case kOpCallObj:
        {
            auto argList = pop();
            translation = std::make_shared<ObjCallNode>(getName(bytecode.obj), std::move(argList));
        }
        break;
    default:
        {
            auto commentText = Lingo::getOpcodeName(bytecode.opID);
            if (bytecode.opcode >= 0x40)
                commentText += " " + std::to_string(bytecode.obj);
            comment = std::make_shared<CommentNode>(commentText);
            translation = std::make_shared<ErrorNode>();
            stack.clear(); // Clear stack so later bytecode won't be too screwed up
        }
    }

    if (comment)
        ast->addStatement(std::move(comment));
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
    std::string res = "on " + name;
    if (argumentNames.size() > 0) {
        res += " ";
        for (size_t i = 0; i < argumentNames.size(); i++) {
            if (i > 0)
                res += ", ";
            res += argumentNames[i];
        }
    }
    res += "\n";
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
        default:
            if (bytecode.opID > 0x40)
                line += " " + std::to_string(bytecode.obj);
            break;
        }
        if (bytecode.translation) {
            line += " ...";
            while (line.length() < 49) {
                line += ".";
            }
            line += " ";
            if (bytecode.translation->isExpression)
                line += "<" + bytecode.translation->toString(true) + ">";
            else
                line += bytecode.translation->toString(true);
        }
        line += "\n";
        res += line;
    }
    res += "end\n";
    return res;
}

}
