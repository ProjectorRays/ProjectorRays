#include <algorithm>

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
        OpCode opcode = kOpRet;
        // instructions can be one, two or three bytes
        auto obj = 0;
        if (op >= 0xc0) {
            opcode = static_cast<OpCode>(op - 0x80);
            obj = stream.readUint24();
        } else if (op >= 0x80) {
            opcode = static_cast<OpCode>(op - 0x40);
            obj = stream.readUint16();
        } else if (op >= 0x40) {
            opcode = static_cast<OpCode>(op);
            obj = stream.readUint8();
        } else {
            opcode = static_cast<OpCode>(op);
        }
        // read the first byte to convert to an opcode
        Bytecode bytecode(opcode, obj, pos);
        bytecodeArray.push_back(bytecode);
        bytecodePosMap[pos] = bytecodeArray.size() - 1;
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

void Handler::readNames(const std::vector<std::string> &names) {
    name = (0 <= nameID && (unsigned)nameID < names.size()) ? names[nameID] : "UNKNOWN";
    for (auto nameID : argumentNameIDs) {
        if (0 <= nameID && (unsigned)nameID < names.size())
            argumentNames.push_back(names[nameID]);
        else
            argumentNames.push_back("UNKNOWN");
    }
    for (auto nameID : localNameIDs) {
        if (0 <= nameID && (unsigned)nameID < names.size())
            localNames.push_back(names[nameID]);
        else
            localNames.push_back("UNKNOWN");
    }
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

void Handler::registerGlobal(std::string name) {
    if (std::find(script->globalNames.begin(), script->globalNames.end(), name) == script->globalNames.end()
            && std::find(globalNames.begin(), globalNames.end(), name) == globalNames.end()) {
        globalNames.push_back(name);
    }
}

void Handler::translate(const std::vector<std::string> &names) {
    stack.clear();
    ast = std::make_unique<AST>(this);
    for (size_t i = 0; i < bytecodeArray.size(); i++) {
        auto &bytecode = bytecodeArray[i];
        auto pos = bytecode.pos;
        if (ast->currentBlock->endPos >= 0) {
            // exit last block if at end
            while (pos == ast->currentBlock->endPos) {
                auto exitedBlock = ast->currentBlock;
                auto blockParent = ast->currentBlock->parent;
                ast->exitBlock();
                if (!blockParent)
                    continue;

                if (blockParent->type == kIfStmtNode) {
                    auto ifStatement = static_cast<IfStmtNode *>(blockParent);
                    if (ifStatement->ifType == kIfElse && exitedBlock == ifStatement->block1.get()) {
                        ast->enterBlock(ifStatement->block2.get());
                    }
                }
            }
        }
        translateBytecode(bytecode, i, names);
    }
}

void Handler::translateBytecode(Bytecode &bytecode, size_t index, const std::vector<std::string> &names) {
    std::shared_ptr<Node> comment = nullptr;
    std::shared_ptr<Node> translation = nullptr;
    BlockNode *nextBlock = nullptr;

    switch (bytecode.opcode) {
    case kOpRet:
        if (index == bytecodeArray.size() - 1) {
            return; // end of handler
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
    case kOpSplitStr:
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
            if (firstChar->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringSplitExprNode>(kChunkChar, std::move(firstChar), std::move(lastChar), std::move(string));
            } else if (firstWord->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringSplitExprNode>(kChunkWord, std::move(firstWord), std::move(lastWord), std::move(string));
            } else if (firstItem->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringSplitExprNode>(kChunkItem, std::move(firstItem), std::move(lastItem), std::move(string));
            } else if (firstLine->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringSplitExprNode>(kChunkLine, std::move(firstLine), std::move(lastLine), std::move(string));
            } else {
                translation = std::make_shared<ErrorNode>();
            }
        }
        break;
    case kOpHiliiteStr:
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
            if (firstChar->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringHiliteStmtNode>(kChunkChar, std::move(firstChar), std::move(lastChar), std::move(string));
            } else if (firstWord->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringHiliteStmtNode>(kChunkWord, std::move(firstWord), std::move(lastWord), std::move(string));
            } else if (firstItem->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringHiliteStmtNode>(kChunkItem, std::move(firstItem), std::move(lastItem), std::move(string));
            } else if (firstLine->getValue()->type != kDatumVoid) {
                translation = std::make_shared<StringHiliteStmtNode>(kChunkLine, std::move(firstLine), std::move(lastLine), std::move(string));
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
    case kOpCastStr:
        {
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
    case kOpPushInt01:
    case kOpPushInt2E:
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
            translation = std::make_shared<LiteralNode>(script->literals[bytecode.obj / variableMultiplier()].value);
            break;
        }
    case kOpPushSymb:
        {
            auto sym = std::make_shared<Datum>(kDatumSymbol, names[bytecode.obj]);
            translation = std::make_shared<LiteralNode>(std::move(sym));
        }
        break;
    case kOpGetGlobal:
        {
            auto name = names[bytecode.obj];
            registerGlobal(name);
            translation = std::make_shared<VarNode>(name);
        }
        break;
    case kOpGetProp:
        translation = std::make_shared<VarNode>(names[bytecode.obj]);
        break;
    case kOpGetParam:
        translation = std::make_shared<VarNode>(argumentNames[bytecode.obj / variableMultiplier()]);
        break;
    case kOpGetLocal:
        translation = std::make_shared<VarNode>(localNames[bytecode.obj / variableMultiplier()]);
        break;
    case kOpSetGlobal:
        {
            auto varName = names[bytecode.obj];
            registerGlobal(varName);
            auto var = std::make_shared<VarNode>(varName);
            auto value = pop();
            translation = std::make_shared<AssignmentStmtNode>(std::move(var), std::move(value));
        }
        break;
    case kOpSetProp:
        {
            auto var = std::make_shared<VarNode>(names[bytecode.obj]);
            auto value = pop();
            translation = std::make_shared<AssignmentStmtNode>(std::move(var), std::move(value));
        }
        break;
    case kOpSetParam:
        {
            auto var = std::make_shared<VarNode>(argumentNames[bytecode.obj / variableMultiplier()]);
            auto value = pop();
            translation = std::make_shared<AssignmentStmtNode>(std::move(var), std::move(value));
        }
        break;
    case kOpSetLocal:
        {
            auto var = std::make_shared<VarNode>(localNames[bytecode.obj / variableMultiplier()]);
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
            auto blockParent = ast->currentBlock->parent;

            if (blockParent && blockParent->type == kIfStmtNode) {
                if (nextBytecode.pos == ast->currentBlock->endPos && targetPrevBytecode.opcode == kOpEndRepeat) {
                    translation = std::make_shared<ExitRepeatStmtNode>();
                } else if (targetBytecode.opcode == kOpEndRepeat) {
                    translation = std::make_shared<NextRepeatStmtNode>();
                } else if (nextBytecode.pos == ast->currentBlock->endPos) {
                    auto ifStmt = static_cast<IfStmtNode *>(blockParent);
                    ifStmt->ifType = kIfElse;
                    ifStmt->block2->endPos = targetPos;
                    return; // if statement amended, nothing to push
                }
            }
        }
        break;
    case kOpEndRepeat:
        {
            auto targetPos = bytecode.pos - bytecode.obj;
            auto i = bytecodePosMap[targetPos];
            while (bytecodeArray[i].opcode != kOpJmpIfZ) i++;
            auto &targetBytecode = bytecodeArray[i];
            auto ifStmt = std::static_pointer_cast<IfStmtNode>(targetBytecode.translation);
            ifStmt->ifType = kRepeatWhile;
            return; // if statement amended, nothing to push
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
            translation = std::make_shared<CallNode>(names[bytecode.obj], std::move(argList));
        }
        break;
    case kOpCallObjOld:
        // Possibly used by old Director versions?
        /* auto object = */ pop();
        /* let argList = */ pop();
        // TODO
        break;
    case kOp59XX:
        pop();
        // TODO
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
        translation = std::make_shared<TheExprNode>(names[bytecode.obj]);
        break;
    case kOpSetMovieProp:
        {
            auto value = pop();
            auto prop = std::make_shared<TheExprNode>(names[bytecode.obj]);
            translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
        }
        break;
    case kOpGetObjProp:
        {
            auto object = pop();
            translation = std::make_shared<ObjPropExprNode>(std::move(object), names[bytecode.obj]);
        }
        break;
    case kOpSetObjProp:
        {
            auto value = pop();
            auto object = pop();
            auto prop = std::make_shared<ObjPropExprNode>(std::move(object), names[bytecode.obj]);
            translation = std::make_shared<AssignmentStmtNode>(std::move(prop), std::move(value));
        }
        break;
    case kOpGetMovieInfo:
        {
            pop(); // FIXME: What is this?
            translation = std::make_shared<TheExprNode>(names[bytecode.obj]);
        }
        break;
    case kOpCallObj:
        {
            auto argList = pop();
            auto &rawList = argList->getValue()->l;
            auto obj = std::move(rawList.front());
            rawList.erase(rawList.begin());
            translation = std::make_shared<ObjCallNode>(std::move(obj), names[bytecode.obj], std::move(argList));
        }
        break;
    default:
        {
            auto commentText = "unk" + std::to_string(bytecode.opcode); // TODO: hex
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
    if (translation->isStatement) {
        ast->addStatement(std::move(translation));
    } else {
        stack.push_back(std::move(translation));
    }

    if (nextBlock)
        ast->enterBlock(nextBlock);
    }
}
