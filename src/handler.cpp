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

std::unique_ptr<Node> Handler::pop() {
    if (stack.empty())
        return std::make_unique<ErrorNode>();
    
    auto res = std::move(stack.back());
    stack.pop_back();
    return res;
}

void Handler::translate(const std::vector<std::string> &names) {
    stack.clear();
    ast = std::make_unique<AST>(name, argumentNames);
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
    std::unique_ptr<Node> comment = nullptr;
    std::unique_ptr<Node> translation = nullptr;
    BlockNode *nextBlock = nullptr;
    bool stmt = false;

    switch (bytecode.opcode) {
    case kOpRet:
        stmt = true;
        translation = std::make_unique<ExitStmtNode>();
        break;
    case kOpPushZero:
        translation = std::make_unique<LiteralNode>(std::make_shared<Datum>(0));
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
            translation = std::make_unique<BinaryOpNode>(bytecode.opcode, std::move(a), std::move(b));
        }
        break;
    case kOpInv:
        {
            auto x = pop();
            translation = std::make_unique<InverseOpNode>(std::move(x));
        }
        break;
    case kOpNot:
        {
            auto x = pop();
            translation = std::make_unique<NotOpNode>(std::move(x));
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
                translation = std::make_unique<StringSplitExprNode>(kChunkChar, std::move(firstChar), std::move(lastChar), std::move(string));
            } else if (firstWord->getValue()->type != kDatumVoid) {
                translation = std::make_unique<StringSplitExprNode>(kChunkWord, std::move(firstWord), std::move(lastWord), std::move(string));
            } else if (firstItem->getValue()->type != kDatumVoid) {
                translation = std::make_unique<StringSplitExprNode>(kChunkItem, std::move(firstItem), std::move(lastItem), std::move(string));
            } else if (firstLine->getValue()->type != kDatumVoid) {
                translation = std::make_unique<StringSplitExprNode>(kChunkLine, std::move(firstLine), std::move(lastLine), std::move(string));
            } else {
                translation = std::make_unique<ErrorNode>();
            }
        }
        break;
    case kOpHiliiteStr:
        {
            stmt = true;
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
                translation = std::make_unique<StringHiliteStmtNode>(kChunkChar, std::move(firstChar), std::move(lastChar), std::move(string));
            } else if (firstWord->getValue()->type != kDatumVoid) {
                translation = std::make_unique<StringHiliteStmtNode>(kChunkWord, std::move(firstWord), std::move(lastWord), std::move(string));
            } else if (firstItem->getValue()->type != kDatumVoid) {
                translation = std::make_unique<StringHiliteStmtNode>(kChunkItem, std::move(firstItem), std::move(lastItem), std::move(string));
            } else if (firstLine->getValue()->type != kDatumVoid) {
                translation = std::make_unique<StringHiliteStmtNode>(kChunkLine, std::move(firstLine), std::move(lastLine), std::move(string));
            } else {
                translation = std::make_unique<ErrorNode>();
            }
        }
        break;
    case kOpOntoSpr:
        {
            auto firstSprite = pop();
            auto secondSprite = pop();
            translation = std::make_unique<SpriteIntersectsExprNode>(std::move(firstSprite), std::move(secondSprite));
        }
        break;
    case kOpIntoSpr:
        {
            auto firstSprite = pop();
            auto secondSprite = pop();
            translation = std::make_unique<SpriteWithinExprNode>(std::move(firstSprite), std::move(secondSprite));
        }
        break;
    case kOpCastStr:
        {
            auto fieldID = pop();
            translation = std::make_unique<FieldExprNode>(std::move(fieldID));
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
            translation = std::move(list);
        }
        break;
    case kOpPushPropList:
        {
            auto list = pop();
            list->getValue()->type = kDatumPropList;
            translation = std::move(list);
        }
        break;
    case kOpPushInt01:
    case kOpPushInt2E:
        {
            auto i = std::make_shared<Datum>((int)bytecode.obj);
            translation = std::make_unique<LiteralNode>(std::move(i));
        }
        break;
    case kOpPushArgListNoRet:
        {
            auto argCount = bytecode.obj;
            std::vector<std::unique_ptr<Node>> args;
            while (argCount) {
                args.insert(args.begin(), pop());
                argCount--;
            }
            auto argList = std::make_shared<Datum>(kDatumArgListNoRet, std::move(args));
            translation = std::make_unique<LiteralNode>(std::move(argList));
        }
        break;
    case kOpPushArgList:
        {
            auto argCount = bytecode.obj;
            std::vector<std::unique_ptr<Node>> args;
            while (argCount) {
                args.insert(args.begin(), pop());
                argCount--;
            }
            auto argList = std::make_shared<Datum>(kDatumArgList, std::move(args));
            translation = std::make_unique<LiteralNode>(std::move(argList));
        }
        break;
    case kOpPushCons:
        {
            auto literalId = script->movie->capitalX ? bytecode.obj : bytecode.obj / 6;
            translation = std::make_unique<LiteralNode>(script->literals[literalId].value);
            break;
        }
    case kOpPushSymb:
        {
            auto sym = std::make_shared<Datum>(kDatumSymbol, names[bytecode.obj]);
            translation = std::make_unique<LiteralNode>(std::move(sym));
        }
        break;
    case kOpGetGlobal:
    case kOpGetProp:
        translation = std::make_unique<VarNode>(names[bytecode.obj]);
        break;
    case kOpGetParam:
        translation = std::make_unique<VarNode>(argumentNames[bytecode.obj]);
        break;
    case kOpGetLocal:
        translation = std::make_unique<VarNode>(localNames[bytecode.obj]);
        break;
    case kOpSetGlobal:
    case kOpSetProp:
        {
            stmt = true;
            auto var = std::make_unique<VarNode>(names[bytecode.obj]);
            auto value = pop();
            translation = std::make_unique<AssignmentStmtNode>(std::move(var), std::move(value));
        }
        break;
    case kOpSetParam:
        {
            stmt = true;
            auto var = std::make_unique<VarNode>(argumentNames[bytecode.obj]);
            auto value = pop();
            translation = std::make_unique<AssignmentStmtNode>(std::move(var), std::move(value));
        }
        break;
    case kOpSetLocal:
        {
            stmt = true;
            auto var = std::make_unique<VarNode>(localNames[bytecode.obj]);
            auto value = pop();
            translation = std::make_unique<AssignmentStmtNode>(std::move(var), std::move(value));
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
                    stmt = true;
                    translation = std::make_unique<ExitRepeatStmtNode>();
                } else if (targetBytecode.opcode == kOpEndRepeat) {
                    stmt = true;
                    translation = std::make_unique<NextRepeatStmtNode>();
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
            auto ifStmt = static_cast<IfStmtNode *>(targetBytecode.translation);
            ifStmt->ifType = kRepeatWhile;
            return; // if statement amended, nothing to push
        }
        break;
    case kOpJmpIfZ:
        {
            stmt = true;
            auto endPos = bytecode.pos + bytecode.obj;
            auto condition = pop();
            auto ifStmt = std::make_unique<IfStmtNode>(std::move(condition));
            ifStmt->block1->endPos = endPos;
            nextBlock = ifStmt->block1.get();
            translation = std::move(ifStmt);
        }
        break;
    case kOpCallLocal:
        {
            auto argList = pop();
            if (argList->getValue()->type == kDatumArgListNoRet)
                stmt = true;
            translation = std::make_unique<CallNode>(script->handlers[bytecode.obj]->name, std::move(argList));
        }
        break;
    case kOpCallExt:
        {
            auto argList = pop();
            if (argList->getValue()->type == kDatumArgListNoRet)
                stmt = true;
            translation = std::make_unique<CallNode>(names[bytecode.obj], std::move(argList));
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
                    translation = std::make_unique<TheExprNode>(propName);
                } else {
                    auto string = pop();
                    auto chunkType = static_cast<ChunkType>(id - 0x0b);
                    translation = std::make_unique<LastStringChunkExprNode>(chunkType, std::move(string));
                }
            }
            break;
        case 0x01:
            {
                auto chunkType = static_cast<ChunkType>(pop()->getValue()->toInt());
                auto string = pop();
                translation = std::make_unique<StringChunkCountExprNode>(chunkType, std::move(string));
            }
            break;
        case 0x02:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto menuID = pop();
                translation = std::make_unique<MenuPropExprNode>(std::move(menuID), propertyID);
            }
            break;
        case 0x03:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto menuID = pop();
                auto itemID = pop();
                translation = std::make_unique<MenuItemPropExprNode>(std::move(menuID), std::move(itemID), propertyID);
            }
            break;
        case 0x04:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto soundID = pop();
                translation = std::make_unique<SoundPropExprNode>(std::move(soundID), propertyID);
            }
            break;
        case 0x06:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto spriteID = pop();
                translation = std::make_unique<SoundPropExprNode>(std::move(spriteID), propertyID);
            }
            break;
        case 0x07:
            {
                auto propertyID = pop()->getValue()->toInt();
                translation = std::make_unique<TheExprNode>(Lingo::getName(Lingo::moviePropertyNames07, propertyID));
            }
            break;
        case 0x08:
            {
                auto propertyID = pop()->getValue()->toInt();
                translation = std::make_unique<TheExprNode>(Lingo::getName(Lingo::moviePropertyNames08, propertyID));
            }
            break;
        case 0x09:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto propName = Lingo::getName(Lingo::castPropertyNames09, propertyID);
                auto castID = pop();
                translation = std::make_unique<CastPropExprNode>(std::move(castID), propName);
            }
            break;
        case 0x0c:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto fieldID = pop();
                translation = std::make_unique<FieldPropExprNode>(std::move(fieldID), propertyID);
            }
            break;
        case 0x0d:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto propName = Lingo::getName(Lingo::castPropertyNames0D, propertyID);
                auto castID = pop();
                translation = std::make_unique<CastPropExprNode>(std::move(castID), propName);
            }
            break;
        default:
            translation = std::make_unique<ErrorNode>();
        }
        break;
    case kOpSet:
        stmt = true;
        switch (bytecode.obj) {
        case 0x00:
            {
                auto id = pop()->getValue()->toInt();
                auto value = pop();
                if (id <= 0x05) {
                    auto propName = Lingo::getName(Lingo::moviePropertyNames00, id);
                    std::unique_ptr<Node> prop = std::make_unique<TheExprNode>(propName);
                    translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
                } else {
                    translation = std::make_unique<ErrorNode>();
                }
            }
            break;
        case 0x03:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto menuID = pop();
                auto itemID = pop();
                auto prop = std::make_unique<MenuItemPropExprNode>(std::move(menuID), std::move(itemID), propertyID);
                translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x04:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto soundID = pop();
                auto prop = std::make_unique<SoundPropExprNode>(std::move(soundID), propertyID);
                translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x06:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto spriteID = pop();
                auto prop = std::make_unique<SoundPropExprNode>(std::move(spriteID), propertyID);
                translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x07:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto prop = std::make_unique<TheExprNode>(Lingo::getName(Lingo::moviePropertyNames07, propertyID));
                translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x09:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto propName = Lingo::getName(Lingo::castPropertyNames09, propertyID);
                auto castID = pop();
                auto prop = std::make_unique<CastPropExprNode>(std::move(castID), propName);
                translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x0c:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto fieldID = pop();
                auto prop = std::make_unique<FieldPropExprNode>(std::move(fieldID), propertyID);
                translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        case 0x0d:
            {
                auto propertyID = pop()->getValue()->toInt();
                auto value = pop();
                auto propName = Lingo::getName(Lingo::castPropertyNames0D, propertyID);
                auto castID = pop();
                auto prop = std::make_unique<CastPropExprNode>(std::move(castID), propName);
                translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
            }
            break;
        default:
            translation = std::make_unique<ErrorNode>();
        }
        break;
    case kOpGetMovieProp:
        translation = std::make_unique<TheExprNode>(names[bytecode.obj]);
        break;
    case kOpSetMovieProp:
        {
            stmt = true;
            auto value = pop();
            auto prop = std::make_unique<TheExprNode>(names[bytecode.obj]);
            translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
        }
        break;
    case kOpGetObjProp:
        {
            auto object = pop();
            translation = std::make_unique<ObjPropExprNode>(std::move(object), names[bytecode.obj]);
        }
        break;
    case kOpSetObjProp:
        {
            stmt = true;
            auto value = pop();
            auto object = pop();
            auto prop = std::make_unique<ObjPropExprNode>(std::move(object), names[bytecode.obj]);
            translation = std::make_unique<AssignmentStmtNode>(std::move(prop), std::move(value));
        }
        break;
    case kOpGetMovieInfo:
        {
            pop(); // FIXME: What is this?
            translation = std::make_unique<TheExprNode>(names[bytecode.obj]);
        }
        break;
    case kOpCallObj:
        {
            auto argList = pop();
            if (argList->getValue()->type == kDatumArgListNoRet)
                stmt = true;
            auto &rawList = argList->getValue()->l;
            auto obj = std::move(rawList.front());
            rawList.erase(rawList.begin(), rawList.begin() + 1);
            translation = std::make_unique<ObjCallNode>(std::move(obj), names[bytecode.obj], std::move(argList));
        }
        break;
    default:
        {
            auto commentText = "unk" + std::to_string(bytecode.opcode); // TODO: hex
            if (bytecode.opcode >= 0x40)
                commentText += " " + std::to_string(bytecode.obj);
            comment = std::make_unique<CommentNode>(commentText);
            translation = std::make_unique<ErrorNode>();
            stack.clear(); // Clear stack so later bytecode won't be too screwed up
        }
    }

    if (comment)
        ast->addStatement(std::move(comment));
    if (!translation)
        translation = std::make_unique<ErrorNode>();

    bytecode.translation = translation.get();
    if (stmt) {
        ast->addStatement(std::move(translation));
    } else {
        stack.push_back(std::move(translation));
    }

    if (nextBlock)
        ast->enterBlock(nextBlock);
    }
}
