#include "lingo.h"
#include "util.h"

namespace ProjectorRays {

/* Lingo */

std::map<uint, std::string> Lingo::opcodeNames = {
    // single-byte
    { kOpRet,               "ret" },
    { kOpRetFactory,        "retfactory" },
    { kOpPushZero,          "pushzero" },
    { kOpMul,               "mul" },
    { kOpAdd,               "add" },
    { kOpSub,               "sub" },
    { kOpDiv,               "div" },
    { kOpMod,               "mod" },
    { kOpInv,               "inv" },
    { kOpJoinStr,           "joinstr" },
    { kOpJoinPadStr,        "joinpadstr" },
    { kOpLt,                "lt" },
    { kOpLtEq,              "lteq" },
    { kOpNtEq,              "nteq" },
    { kOpEq,                "eq" },
    { kOpGt,                "gt" },
    { kOpGtEq,              "gteq" },
    { kOpAnd,               "and" },
    { kOpOr,                "or" },
    { kOpNot,               "not" },
    { kOpContainsStr,       "containsstr" },
    { kOpContains0Str,      "contains0str" },
    { kOpGetChunk,          "getchunk" },
    { kOpHiliteChunk,       "hilitechunk" },
    { kOpOntoSpr,           "ontospr" },
    { kOpIntoSpr,           "intospr" },
    { kOpGetField,          "getfield" },
    { kOpStartTell,         "starttell" },
    { kOpEndTell,           "endtell" },
    { kOpPushList,          "pushlist" },
    { kOpPushPropList,      "pushproplist" },

    // multi-byte
    { kOpPushInt8,          "pushint8" },
    { kOpPushArgListNoRet,  "pusharglistnoret" },
    { kOpPushArgList,       "pusharglist" },
    { kOpPushCons,          "pushcons" },
    { kOpPushSymb,          "pushsymb" },
    { kOpPushVarRef,        "pushvarref" },
    { kOpGetGlobal2,        "getglobal2" },
    { kOpGetGlobal,         "getglobal" },
    { kOpGetProp,           "getprop" },
    { kOpGetParam,          "getparam" },
    { kOpGetLocal,          "getlocal" },
    { kOpSetGlobal2,        "setglobal2" },
    { kOpSetGlobal,         "setglobal" },
    { kOpSetProp,           "setprop" },
    { kOpSetParam,          "setparam" },
    { kOpSetLocal,          "setlocal" },
    { kOpJmp,               "jmp" },
    { kOpEndRepeat,         "endrepeat" },
    { kOpJmpIfZ,            "jmpifz" },
    { kOpLocalCall,         "localcall" },
    { kOpExtCall,           "extcall" },
    { kOpObjCallV4,         "objcallv4" },
    { kOpPut,               "put" },
    { kOpPutChunk,          "putchunk" },
    { kOpDeleteChunk,       "deletechunk" },
    { kOpGet,               "get" },
    { kOpSet,               "set" },
    { kOpGetMovieProp,      "getmovieprop" },
    { kOpSetMovieProp,      "setmovieprop" },
    { kOpGetObjProp,        "getobjprop" },
    { kOpSetObjProp,        "setobjprop" },
    { kOpTellCall,          "tellcall" },
    { kOpPeek,              "peek" },
    { kOpPop,               "pop" },
    { kOpTheBuiltin,        "thebuiltin" },
    { kOpObjCall,           "objcall" },
    { kOpPushInt16,         "pushint16" },
    { kOpPushInt32,         "pushint32" },
    { kOpGetChainedProp,    "getchainedprop" },
    { kOpPushFloat32,       "pushfloat32" },
    { kOpGetTopLevelProp,   "gettoplevelprop" }
};

std::map<uint, std::string> Lingo::binaryOpNames = {
    { kOpMul,           "*" },
    { kOpAdd,           "+" },
    { kOpSub,           "-" },
    { kOpDiv,           "/" },
    { kOpMod,           "mod" },
    { kOpJoinStr,       "&" },
    { kOpJoinPadStr,    "&&" },
    { kOpLt,            "<" },
    { kOpLtEq,          "<=" },
    { kOpNtEq,          "<>" },
    { kOpEq,            "=" },
    { kOpGt,            ">" },
    { kOpGtEq,          ">=" },
    { kOpAnd,           "and" },
    { kOpOr,            "or" },
    { kOpContainsStr,   "contains" },
    { kOpContains0Str,  "starts" }
};

std::map<uint, std::string> Lingo::chunkTypeNames = {
    { kChunkChar, "char" },
    { kChunkWord, "word" },
    { kChunkItem, "item" },
    { kChunkLine, "line" }
};

std::map<uint, std::string> Lingo::putTypeNames = {
    { kPutInto,     "into" },
    { kPutAfter,    "after" },
    { kPutBefore,   "before" }
};

std::map<uint, std::string> Lingo::moviePropertyNames00 = {
    { 0x00, "floatPrecision" },
    { 0x01, "mouseDownScript" },
    { 0x02, "mouseUpScript" },
    { 0x03, "keyDownScript" },
    { 0x04, "keyUpScript" },
    { 0x05, "timeoutScript" },
    { 0x06, "short time" },
    { 0x07, "abbr time" },
    { 0x08, "long time" },
    { 0x09, "short date" },
    { 0x0a, "abbr date" },
    { 0x0b, "long date" }
};

std::map<uint, std::string> Lingo::menuPropertyNames = {
    { 0x01, "name" },
    { 0x02, "number of menuItems" }
};

std::map<uint, std::string> Lingo::menuItemPropertyNames = {
    { 0x01, "name" },
    { 0x02, "checkMark" },
    { 0x03, "enabled" },
    { 0x04, "script" }
};

std::map<uint, std::string> Lingo::soundPropertyNames = {
    { 0x01, "volume" }
};

std::map<uint, std::string> Lingo::spritePropertyNames = {
    { 0x01, "type" },
    { 0x02, "backColor" },
    { 0x03, "bottom" },
    { 0x04, "castNum" },
    { 0x05, "constraint" },
    { 0x06, "cursor" },
    { 0x07, "foreColor" },
    { 0x08, "height" },
    { 0x09, "immediate" },
    { 0x0a, "ink" },
    { 0x0b, "left" },
    { 0x0c, "lineSize" },
    { 0x0d, "locH" },
    { 0x0e, "locV" },
    { 0x0f, "movieRate" },
    { 0x10, "movieTime" },
    { 0x11, "pattern" },
    { 0x12, "puppet" },
    { 0x13, "right" },
    { 0x14, "startTime" },
    { 0x15, "stopTime" },
    { 0x16, "stretch" },
    { 0x17, "top" },
    { 0x18, "trails" },
    { 0x19, "visible" },
    { 0x1a, "volume" },
    { 0x1b, "width" },
    { 0x1c, "blend" },
    { 0x1d, "scriptNum" },
    { 0x1e, "moveableSprite" },
    { 0x1f, "editableText" },
    { 0x20, "scoreColor" },
    { 0x21, "loc" },
    { 0x22, "rect" }
};

std::map<uint, std::string> Lingo::moviePropertyNames07 = {
    { 0x01, "beepOn" },
    { 0x02, "buttonStyle" },
    { 0x03, "centerStage" },
    { 0x04, "checkBoxAccess" },
    { 0x05, "checkboxType" },
    { 0x06, "colorDepth" },
    { 0x07, "colorQD" },
    { 0x08, "exitLock" },
    { 0x09, "fixStageSize" },
    { 0x0a, "fullColorPermit" },
    { 0x0b, "imageDirect" },
    { 0x0c, "doubleClick" },
//  { 0x0d, ??? },
    { 0x0e, "lastClick" },
    { 0x0f, "lastEvent" },
//  { 0x10, ??? },
    { 0x11, "lastKey" },
    { 0x12, "lastRoll"},
    { 0x13, "timeoutLapsed" },
    { 0x14, "multiSound" },
    { 0x15, "pauseState" },
    { 0x16, "quickTimePresent" },
    { 0x17, "selEnd" },
    { 0x18, "selStart" },
    { 0x19, "soundEnabled" },
    { 0x1a, "soundLevel" },
    { 0x1b, "stageColor" },
//  { 0x1c, ??? },
    { 0x1d, "switchColorDepth" },
    { 0x1e, "timeoutKeyDown" },
    { 0x1f, "timeoutLength" },
    { 0x20, "timeoutMouse" },
    { 0x21, "timeoutPlay" },
    { 0x22, "timer" },
    { 0x23, "preLoadRAM" }
};

std::map<uint, std::string> Lingo::moviePropertyNames08 = {
    { 0x01, "perFrameHook" },
    { 0x02, "number of castMembers" },
    { 0x03, "number of menus" }
};

std::map<uint, std::string> Lingo::memberPropertyNames = {
    { 0x01, "name" },
    { 0x02, "text" },
    { 0x03, "textStyle" },
    { 0x04, "textFont" },
    { 0x05, "textHeight" },
    { 0x06, "textAlign" },
    { 0x07, "textSize" },
    { 0x08, "picture" },
    { 0x09, "hilite" },
    { 0x0a, "number" },
    { 0x0b, "size" },
    { 0x0c, "loop" },
    { 0x0d, "duration" },
    { 0x0e, "controller" },
    { 0x0f, "directToStage" },
    { 0x10, "sound" },
    { 0x11, "foreColor" },
    { 0x12, "backColor" }
};

std::string Lingo::getOpcodeName(uint8_t id) {
    if (id >= 0x40)
        id = 0x40 + id % 0x40;
    auto it = opcodeNames.find(id);
    if (it == opcodeNames.end()){
        char hexID[3];
        sprintf(hexID, "%02X", id);
        std::string res = "unk";
        res.append(hexID);
        return res;
    }
    return it->second;
}

std::string Lingo::getName(const std::map<uint, std::string> &nameMap, uint id) {
    auto it = nameMap.find(id);
    if (it == nameMap.end())
        return "ERROR";
    return it->second;
}

/* Datum */

int Datum::toInt() {
    switch (type) {
    case kDatumInt:
        return i;
    case kDatumFloat:
        return f;
    default:
        break;
    }
    return 0;
}

std::string Datum::toString(bool dot, bool sum) {
    switch (type) {
    case kDatumVoid:
        return "VOID";
    case kDatumSymbol:
        return "#" + s;
    case kDatumVarRef:
        return s;
    case kDatumString:
        if (s.length() == 1) {
            switch (s[0]) {
            case '\x03':
                return "ENTER";
            case '\x08':
                return "BACKSPACE";
            case '\t':
                return "TAB";
            case '\r':
                return "RETURN";
            case '"':
                return "QUOTE";
            default:
                break;
            }
        }
        return "\"" + s + "\"";
    case kDatumInt:
        return std::to_string(i);
    case kDatumFloat:
        return floatToString(f);
    case kDatumList:
    case kDatumArgList:
    case kDatumArgListNoRet:
        {
            std::string res = "";
            if (type == kDatumList)
                res += "[";
            for (size_t i = 0; i < l.size(); i++) {
                if (i > 0)
                    res += ", ";
                res += l[i]->toString(dot, sum);
            }
            if (type == kDatumList)
                res += "]";
            return res;
        }
    case kDatumPropList:
        {
            std::string res = "[";
            if (l.size() == 0) {
                res += ":";
            } else {
                for (size_t i = 0; i < l.size(); i += 2) {
                    if (i > 0)
                        res += ", ";
                    res += l[i]->toString(dot, sum) + ": " + l[i + 1]->toString(dot, sum);
                }
            }
            res += "]";
            return res;
        }
    }

    return "ERROR";
}

void to_json(ordered_json &j, const Datum &c) {
    switch (c.type) {
    case kDatumString:
        j = c.s;
        break;
    case kDatumInt:
        j = c.i;
        break;
    case kDatumFloat:
        j = c.f;
        break;
    default:
        j = nullptr;
        break;
    }
}

/* AST */

std::string AST::toString(bool dot, bool sum) {
    return root->toString(dot, sum);
}

void AST::addStatement(std::shared_ptr<Node> statement) {
    currentBlock->addChild(std::move(statement));
}

void AST::enterBlock(BlockNode *block) {
    currentBlock = block;
}

void AST::exitBlock() {
    auto ancestorStatement = currentBlock->ancestorStatement();
    if (!ancestorStatement) {
        currentBlock = nullptr;
        return;
    }

    auto block = ancestorStatement->parent;
    if (!block || block->type != kBlockNode) {
        currentBlock = nullptr;
        return;
    }

    currentBlock = static_cast<BlockNode *>(block);
}

/* Node */

std::string Node::toString(bool dot, bool sum) {
    return "";
}

std::shared_ptr<Datum> Node::getValue() {
    return std::make_shared<Datum>();
}

Node *Node::ancestorStatement() {
    Node *ancestor = parent;
    while (ancestor && !ancestor->isStatement) {
        ancestor = ancestor->parent;
    }
    return ancestor;
}

LoopNode *Node::ancestorLoop() {
    Node *ancestor = parent;
    while (ancestor && !ancestor->isLoop) {
        ancestor = ancestor->parent;
    }
    return static_cast<LoopNode *>(ancestor);
}

/* ErrorNode */

std::string ErrorNode::toString(bool dot, bool sum) {
    return "ERROR";
}

/* TempNode */

std::string TempNode::toString(bool dot, bool sum) {
    return "TEMP";
}

/* CommentNode */

std::string CommentNode::toString(bool dot, bool sum) {
    return "-- " + text;
}

/* LiteralNode */

std::string LiteralNode::toString(bool dot, bool sum) {
    return value->toString(dot, sum);
}

std::shared_ptr<Datum> LiteralNode::getValue() {
    return value;
}

/* BlockNode */

std::string BlockNode::toString(bool dot, bool sum) {
    std::string res = "";
    for (const auto &child : children) {
        res += indent(child->toString(dot, sum) + "\n");
    }
    return res;
}

void BlockNode::addChild(std::shared_ptr<Node> child) {
    child->parent = this;
    children.push_back(std::move(child));
}

/* HandlerNode */

std::string HandlerNode::toString(bool dot, bool sum) {
    std::string res = "on " + handler->name;
    if (handler->argumentNames.size() > 0) {
        res += " ";
        for (size_t i = 0; i < handler->argumentNames.size(); i++) {
            if (i > 0)
                res += ", ";
            res += handler->argumentNames[i];
        }
    }
    res += "\n";
    if (handler->globalNames.size() > 0) {
        res += "  global ";
        for (size_t i = 0; i < handler->globalNames.size(); i++) {
            if (i > 0)
                res += ", ";
            res += handler->globalNames[i];
        }
        res += "\n";
    }
    res += block->toString(dot, sum);
    res += "end\n";
    return res;
}

/* ExitStmtNode */

std::string ExitStmtNode::toString(bool dot, bool sum) {
    return "exit";
}

/* InverseOpNode */

std::string InverseOpNode::toString(bool dot, bool sum) {
    return "-" + operand->toString(dot, sum);
}

/* NotOpNode */

std::string NotOpNode::toString(bool dot, bool sum) {
    return "not " + operand->toString(dot, sum);
}

/* BinaryOpNode */

std::string BinaryOpNode::toString(bool dot, bool sum) {
    auto opString = Lingo::getName(Lingo::binaryOpNames, opcode);
    return left->toString(dot, sum) + " " +  opString + " " + right->toString(dot, sum);
}

/* ChunkExprNode */

std::string ChunkExprNode::toString(bool dot, bool sum) {
    auto res = Lingo::getName(Lingo::chunkTypeNames, type) + " " + first->toString(dot, sum);
    if (!(last->type == kLiteralNode && last->getValue()->type == kDatumInt && last->getValue()->i == 0)) {
        res += " to " + last->toString(dot, sum);
    }
    // we want the string to always be verbose
    res += " of " + string->toString(false, sum);
    return res;
}

/* ChunkHiliteStmtNode */

std::string ChunkHiliteStmtNode::toString(bool dot, bool sum) {
    return "hilite " + chunk->toString(dot, sum);
}

/* ChunkDeleteStmtNode */

std::string ChunkDeleteStmtNode::toString(bool dot, bool sum) {
    return "delete " + chunk->toString(dot, sum);
}

/* SpriteIntersectsExprNode */

std::string SpriteIntersectsExprNode::toString(bool dot, bool sum) {
    return "sprite " + firstSprite->toString(dot, sum) + " intersects " + secondSprite->toString(dot, sum);
}

/* SpriteWithinExprNode */

std::string SpriteWithinExprNode::toString(bool dot, bool sum) {
    return "sprite " + firstSprite->toString(dot, sum) + " within " + secondSprite->toString(dot, sum);
}

/* MemberExprNode */

std::string MemberExprNode::toString(bool dot, bool sum) {
    std::string res = type;
    if (!castID || (castID->type == kLiteralNode && castID->getValue()->type == kDatumInt && castID->getValue()->i == 0)) {
        if (dot) {
            res += "(" + memberID->toString(dot, sum) + ")";
        } else {
            res += " " + memberID->toString(dot, sum);
        }
    } else {
        res += "(" + memberID->toString(dot, sum) + ", " + castID->toString(dot, sum) + ")";
    }
    return res;
}

/* VarNode */

std::string VarNode::toString(bool dot, bool sum) {
    return varName;
}

/* AssignmentStmtNode */

std::string AssignmentStmtNode::toString(bool dot, bool sum) {
    if (!dot || forceVerbose) {
        // we want the variable to always be verbose
        return "set " + variable->toString(false, sum) + " to " + value->toString(dot, sum);
    }
    
    return variable->toString(dot, sum) + " = " + value->toString(dot, sum);
}

/* IfStmtNode */

std::string IfStmtNode::toString(bool dot, bool sum) {
    std::string res = "if " + condition->toString(dot, sum) + " then";
    if (sum) {
        if (hasElse) {
            res += " / else";
        }
    } else {
        res += "\n";
        res += block1->toString(dot, sum);
        if (hasElse) {
            res += "else\n" + block2->toString(dot, sum);
        }
        res += "end if";
    }
    return res;
}

/* RepeatWhileStmtNode */

std::string RepeatWhileStmtNode::toString(bool dot, bool sum) {
    std::string res = "repeat while " + condition->toString(dot, sum);
    if (!sum) {
        res += "\n" + block->toString(dot, sum) + "end repeat";
    }
    return res;
}

/* RepeatWithInStmtNode */

std::string RepeatWithInStmtNode::toString(bool dot, bool sum) {
    std::string res = "repeat with " + varName + " in " + list->toString(dot, sum);
    if (!sum) {
        res += "\n" + block->toString(dot, sum) + "end repeat";
    }
    return res;
}

/* RepeatWithToStmtNode */

std::string RepeatWithToStmtNode::toString(bool dot, bool sum) {
    std::string res = "repeat with " + varName + " = " + start->toString(dot, sum);
    if (up) {
        res += " to ";
    } else {
        res += " down to ";
    }
    res += end->toString(dot, sum);
    if (!sum) {
        res += "\n" + block->toString(dot, sum) + "end repeat";
    }
    return res;
}

/* CaseNode */

std::string CaseNode::toString(bool dot, bool sum) {
    std::string res;
    if (sum) {
        res += "(case) ";
        if (parent->type == kCaseNode) {
            auto parentCase = static_cast<CaseNode *>(parent);
            if (parentCase->nextOr.get() == this) {
                res += "..., ";
            }
        }
        res += value->toString(dot, sum);
        if (nextOr) {
            res += ", ...";
        } else {
            res += ":";
        }
    } else {
        res += value->toString(dot, sum);
        if (nextOr) {
            res += ", " + nextOr->toString(dot, sum);
        } else {
            res += ":\n" + block->toString(dot, sum);
        }
        if (nextCase) {
            res += nextCase->toString(dot, sum);
        } else if (otherwise) {
            res += "otherwise:\n" + otherwise->toString(dot, sum);
        }
    }
    return res;
}

/* CasesStmtNode */

std::string CasesStmtNode::toString(bool dot, bool sum) {
    std::string res = "case " + value->toString(dot, sum) + " of";
    if (!sum) {
        res += "\n" + indent(firstCase->toString(dot, sum)) + "end case";
    }
    return res;
}

/* TellStmtNode */

std::string TellStmtNode::toString(bool dot, bool sum) {
    std::string res = "tell " + window->toString(dot, sum);
    if (!sum) {
        res += "\n" + block->toString(dot, sum) + "end tell";
    }
    return res;
}

/* CallNode */

bool CallNode::noParens() {
    if (isStatement) {
        // TODO: Make a complete list of commonly paren-less commands
        if (name == "put")
            return true;
        if (name == "return")
            return true;
    }

    return false;
}

std::string CallNode::toString(bool dot, bool sum) {
    if (isExpression && argList->getValue()->l.size() == 0) {
        if (name == "pi")
            return "PI";
        if (name == "space")
            return "SPACE";
        if (name == "void")
            return "VOID";
    }

    if (noParens())
        return name + " " + argList->toString(dot, sum);

    return name + "(" + argList->toString(dot, sum) + ")";
}

/* ObjCallNode */

std::string ObjCallNode::toString(bool dot, bool sum) {
    auto rawArgs = argList->getValue()->l;
    std::string res = rawArgs[0]->toString(dot, sum) + "." + name + "(";
    for (size_t i = 1; i < rawArgs.size(); i++) {
        if (i > 1)
            res += ", ";
        res += rawArgs[i]->toString(dot, sum);
    }
    res += ")";
    return res;
}

/* ObjCallV4Node */

std::string ObjCallV4Node::toString(bool dot, bool sum) {
    return obj->toString(dot, sum) + "(" + argList->toString(dot, sum) + ")";
}

/* TheExprNode */

std::string TheExprNode::toString(bool dot, bool sum) {
    return "the " + prop;
}

/* LastStringChunkExprNode */

std::string LastStringChunkExprNode::toString(bool dot, bool sum) {
    auto typeString = Lingo::getName(Lingo::chunkTypeNames, type);
    // we want the string to always be verbose
    return "the last " + typeString + " in " + string->toString(false, sum);
}

/* StringChunkCountExprNode */

std::string StringChunkCountExprNode::toString(bool dot, bool sum) {
    auto typeString = Lingo::getName(Lingo::chunkTypeNames, type);
    // we want the string to always be verbose
    return "the number of " + typeString + "s in " + string->toString(false, sum);
}

/* MenuPropExprNode */

std::string MenuPropExprNode::toString(bool dot, bool sum) {
    auto propString = Lingo::getName(Lingo::menuPropertyNames, prop);
    return "the " + propString + " of menu " + menuID->toString(dot, sum);
}

/* MenuItemPropExprNode */

std::string MenuItemPropExprNode::toString(bool dot, bool sum) {
    auto propString = Lingo::getName(Lingo::menuItemPropertyNames, prop);
    return "the " + propString + " of menuItem " + itemID->toString(dot, sum) + " of menu " + menuID->toString(dot, sum);
}

/* SoundPropExprNode */

std::string SoundPropExprNode::toString(bool dot, bool sum) {
    auto propString = Lingo::getName(Lingo::soundPropertyNames, prop);
    return "the " + propString + " of sound " + soundID->toString(dot, sum);
}

/* SpritePropExprNode */

std::string SpritePropExprNode::toString(bool dot, bool sum) {
    auto propString = Lingo::getName(Lingo::spritePropertyNames, prop);
    return "the " + propString + " of sprite " + spriteID->toString(dot, sum);
}

/* ThePropExprNode */

std::string ThePropExprNode::toString(bool dot, bool sum) {
    // we want the object to always be verbose
    return "the " + prop + " of " + obj->toString(false, sum);
}

/* ObjPropExprNode */

std::string ObjPropExprNode::toString(bool dot, bool sum) {
    if (dot)
        return obj->toString(dot, sum) + "." + prop;
    
    return "the " + prop + " of " + obj->toString(dot, sum);
}

/* ObjBracketExprNode */

std::string ObjBracketExprNode::toString(bool dot, bool sum) {
    return obj->toString(dot, sum) + "[" + prop->toString(dot, sum) + "]";
}

/* ObjPropIndexExprNode */

std::string ObjPropIndexExprNode::toString(bool dot, bool sum) {
    std::string res = obj->toString(dot, sum) + "." + prop + "[" + index->toString(dot, sum);
    if (index2)
        res += ".." + index2->toString(dot, sum);
    res += "]";
    return res;
}

/* ExitRepeatStmtNode */

std::string ExitRepeatStmtNode::toString(bool dot, bool sum) {
    return "exit repeat";
}

/* NextRepeatStmtNode */

std::string NextRepeatStmtNode::toString(bool dot, bool sum) {
    return "next repeat";
}

/* PutStmtNode */

std::string PutStmtNode::toString(bool dot, bool sum) {
    auto typeString = Lingo::getName(Lingo::putTypeNames, type);
    // we want the variable to always be verbose
    return "put " + value->toString(dot, sum) + " " + typeString + " " + variable->toString(false, sum);
}

}
