#include <iomanip>
#include <limits>
#include <sstream>

#include "lingo.h"
#include "util.h"

namespace ProjectorRays {

/* Lingo */

std::map<uint, std::string> Lingo::opcodeNames = {
    // single-byte
    { kOpRet,               "ret" },
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
    { kOpStartObj,          "startobj" },
    { kOpStopObj,           "stopobj" },
    { kOpPushList,          "pushlist" },
    { kOpPushPropList,      "pushproplist" },

    // multi-byte
    { kOpPushInt8,          "pushint8" },
    { kOpPushArgListNoRet,  "pusharglistnoret" },
    { kOpPushArgList,       "pusharglist" },
    { kOpPushCons,          "pushcons" },
    { kOpPushSymb,          "pushsymb" },
    { kOpPushVarRef,        "pushvarref" },
    { kOpGetGlobal,         "getglobal" },
    { kOpGetProp,           "getprop" },
    { kOpGetParam,          "getparam" },
    { kOpGetLocal,          "getlocal" },
    { kOpSetGlobal,         "setglobal" },
    { kOpSetProp,           "setprop" },
    { kOpSetParam,          "setparam" },
    { kOpSetLocal,          "setlocal" },
    { kOpJmp,               "jmp" },
    { kOpEndRepeat,         "endrepeat" },
    { kOpJmpIfZ,            "jmpifz" },
    { kOpCallLocal,         "localcall" },
    { kOpCallExt,           "extcall" },
    { kOpCallObjOld,        "oldobjcall" },
    { kOpPut,               "put" },
    { kOp5BXX,              "op5Bxx" },
    { kOpGet,               "get" },
    { kOpSet,               "set" },
    { kOpGetMovieProp,      "getmovieprop" },
    { kOpSetMovieProp,      "setmovieprop" },
    { kOpGetObjProp,        "getobjprop" },
    { kOpSetObjProp,        "setobjprop" },
    { kOpPeek,              "peek" },
    { kOpPop,               "pop" },
    { kOpGetMovieInfo,      "getmovieinfo" },
    { kOpCallObj,           "objcall" },
    { kOpPushInt16,         "pushint16" },
    { kOpPushInt32,         "pushint32" },
    { kOpGetChainedProp,    "getchainedprop" },
    { kOpPushFloat32,       "pushfloat32" }
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
    { 0x0a, "ink" },
    { 0x0b, "left" },
    { 0x0c, "lineSize" },
    { 0x0d, "locH" },
    { 0x0e, "locV" },
    { 0x0f, "movieRate" },
    { 0x10, "movieTime" },
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
    { 0x1d, "scriptNum" },
    { 0x1e, "moveableSprite" },
    { 0x20, "scoreColor" }
};

std::map<uint, std::string> Lingo::moviePropertyNames07 = {
    { 0x01, "beepOn" },
    { 0x02, "buttonStyle" },
    { 0x03, "centerStage" },
    { 0x04, "checkBoxAccess" },
    { 0x05, "checkboxType" },
    { 0x06, "colorDepth" },
    { 0x08, "exitLock" },
    { 0x09, "fixStageSize" },
    { 0x13, "timeoutLapsed" },
    { 0x17, "selEnd" },
    { 0x18, "selStart" },
    { 0x19, "soundEnabled" },
    { 0x1a, "soundLevel" },
    { 0x1b, "stageColor" },
    { 0x1d, "stillDown" },
    { 0x1e, "timeoutKeyDown" },
    { 0x1f, "timeoutLength" },
    { 0x20, "timeoutMouse" },
    { 0x21, "timeoutPlay" },
    { 0x22, "timer" }
};

std::map<uint, std::string> Lingo::moviePropertyNames08 = {
    { 0x01, "perFrameHook" },
    { 0x02, "number of castMembers" },
    { 0x03, "number of menus" }
};

std::map<uint, std::string> Lingo::castPropertyNames09 = {
    { 0x01, "name" },
    { 0x02, "text" },
    { 0x08, "picture" },
    { 0x0a, "number" },
    { 0x0b, "size" },
    { 0x11, "foreColor" },
    { 0x12, "backColor" }
};

std::map<uint, std::string> Lingo::fieldPropertyNames = {
    { 0x03, "textStyle" },
    { 0x04, "textFont" },
    { 0x05, "textHeight" },
    { 0x06, "textAlign" },
    { 0x07, "textSize" }
};

std::map<uint, std::string> Lingo::castPropertyNames0D = {
    { 0x01, "sound" }
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

std::string Datum::toString(bool summary) {
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
        {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << f;
            std::string res = ss.str();
            while (res[res.size() - 1] == '0' && res[res.size() - 2] != '.') {
                res.pop_back();
            }
            return res;
        }
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
                res += l[i]->toString(summary);
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
                    res += l[i]->toString(summary) + ": " + l[i + 1]->toString(summary);
                }
            }
            res += "]";
            return res;
        }
    }

    return "ERROR";
}

/* AST */

std::string AST::toString(bool summary) {
    return root->toString(summary);
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

std::string Node::toString(bool summary) {
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

/* ErrorNode */

std::string ErrorNode::toString(bool summary) {
    return "ERROR";
}

/* TempNode */

std::string TempNode::toString(bool summary) {
    return "TEMP";
}

/* CommentNode */

std::string CommentNode::toString(bool summary) {
    return "-- " + text;
}

/* LiteralNode */

std::string LiteralNode::toString(bool summary) {
    return value->toString(summary);
}

std::shared_ptr<Datum> LiteralNode::getValue() {
    return value;
}

/* BlockNode */

std::string BlockNode::toString(bool summary) {
    std::string res = "";
    for (const auto &child : children) {
        res += indent(child->toString(summary) + "\n");
    }
    return res;
}

void BlockNode::addChild(std::shared_ptr<Node> child) {
    child->parent = this;
    children.push_back(std::move(child));
}

/* HandlerNode */

std::string HandlerNode::toString(bool summary) {
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
    res += block->toString(summary);
    res += "end\n";
    return res;
}

/* ExitStmtNode */

std::string ExitStmtNode::toString(bool summary) {
    return "exit";
}

/* InverseOpNode */

std::string InverseOpNode::toString(bool summary) {
    return "-" + operand->toString(summary);
}

/* NotOpNode */

std::string NotOpNode::toString(bool summary) {
    return "not " + operand->toString(summary);
}

/* BinaryOpNode */

std::string BinaryOpNode::toString(bool summary) {
    auto opString = Lingo::getName(Lingo::binaryOpNames, opcode);
    return left->toString(summary) + " " +  opString + " " + right->toString(summary);
}

/* ChunkExprNode */

std::string ChunkExprNode::toString(bool summary) {
    auto res = Lingo::getName(Lingo::chunkTypeNames, type) + " " + first->toString(summary);
    if (last->getValue()->toInt()) {
        res += " to " + last->toString(summary);
    }
    res += " of " + string->toString(summary);
    return res;
}

/* ChunkHiliteStmtNode */

std::string ChunkHiliteStmtNode::toString(bool summary) {
    auto res = "hilite " + Lingo::getName(Lingo::chunkTypeNames, type) + " " + first->toString(summary);
    if (last->getValue()->toInt()) {
        res += " to " + last->toString(summary);
    }
    res += " of field " + fieldID->toString(summary);
    return res;
}

/* SpriteIntersectsExprNode */

std::string SpriteIntersectsExprNode::toString(bool summary) {
    return "sprite(" + firstSprite->toString(summary) + ").intersects(" + secondSprite->toString(summary) + ")";
}

/* SpriteWithinExprNode */

std::string SpriteWithinExprNode::toString(bool summary) {
    return "sprite(" + firstSprite->toString(summary) + ").within(" + secondSprite->toString(summary) + ")";
}

/* FieldExprNode */

std::string FieldExprNode::toString(bool summary) {
    return "field(" + fieldID->toString(summary) + ")";
}

/* VarNode */

std::string VarNode::toString(bool summary) {
    return varName;
}

/* AssignmentStmtNode */

std::string AssignmentStmtNode::toString(bool summary) {
    return variable->toString(summary) + " = " + value->toString(summary);
}

/* IfStmtNode */

std::string IfStmtNode::toString(bool summary) {
    std::string res;
    switch (ifType) {
    case kIf:
        res = "if " + condition->toString(summary) + " then";
        break;
    case kIfElse:
        res = "if " + condition->toString(summary) + " then";
        break;
    case kRepeatWhile:
        res = "repeat while " + condition->toString(summary);
        break;
    }
    if (summary) {
        if (ifType == kIfElse) {
            res += " / else";
        }
    } else {
        res += "\n";
        res += block1->toString(summary);
        if (ifType == kIfElse) {
            res += "else\n" + block2->toString(summary);
        }
        if (ifType == kRepeatWhile) {
            res += "end repeat";
        } else {
            res += "end if";
        }
    }
    return res;
}

/* RepeatWithInStmtNode */

std::string RepeatWithInStmtNode::toString(bool summary) {
    std::string res = "repeat with " + varName + " in " + list->toString(summary);
    if (!summary) {
        res += "\n" + block->toString(summary) + "end repeat";
    }
    return res;
}

/* CaseNode */

std::string CaseNode::toString(bool summary) {
    std::string res;
    if (summary) {
        res += "(case) ";
        if (parent->type == kCaseNode) {
            auto parentCase = static_cast<CaseNode *>(parent);
            if (parentCase->nextOr.get() == this) {
                res += "..., ";
            }
        }
        res += value->toString(summary);
        if (nextOr) {
            res += ", ...";
        } else {
            res += ":";
        }
    } else {
        res += value->toString(summary);
        if (nextOr) {
            res += ", " + nextOr->toString(summary);
        } else {
            res += ":\n" + block->toString(summary);
        }
        if (nextCase) {
            res += nextCase->toString(summary);
        } else if (otherwise) {
            res += "otherwise:\n" + otherwise->toString(summary);
        }
    }
    return res;
}

/* CasesStmtNode */

std::string CasesStmtNode::toString(bool summary) {
    std::string res = "case " + value->toString(summary) + " of";
    if (!summary) {
        res += "\n" + indent(firstCase->toString(summary)) + "end case";
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

std::string CallNode::toString(bool summary) {
    if (isExpression && argList->getValue()->l.size() == 0) {
        if (name == "pi")
            return "PI";
        if (name == "space")
            return "SPACE";
        if (name == "void")
            return "VOID";
    }

    if (noParens())
        return name + " " + argList->toString(summary);

    return name + "(" + argList->toString(summary) + ")";
}

/* ObjCallNode */

std::string ObjCallNode::toString(bool summary) {
    auto rawArgs = argList->getValue()->l;
    std::string res = rawArgs[0]->toString(summary) + "." + name + "(";
    for (size_t i = 1; i < rawArgs.size(); i++) {
        if (i > 1)
            res += ", ";
        res += rawArgs[i]->toString(summary);
    }
    res += ")";
    return res;
}

/* TheExprNode */

std::string TheExprNode::toString(bool summary) {
    return "the " + prop;
}

/* LastStringChunkExprNode */

std::string LastStringChunkExprNode::toString(bool summary) {
    auto typeString = Lingo::getName(Lingo::chunkTypeNames, type);
    return "the last " + typeString + " in " + string->toString(summary);
}

/* StringChunkCountExprNode */

std::string StringChunkCountExprNode::toString(bool summary) {
    auto typeString = Lingo::getName(Lingo::chunkTypeNames, type);
    return "the number of " + typeString + " in " + string->toString(summary);
}

/* MenuPropExprNode */

std::string MenuPropExprNode::toString(bool summary) {
    auto propString = Lingo::getName(Lingo::menuPropertyNames, prop);
    return "menu(" + menuID->toString(summary) + ")." + propString;
}

/* MenuItemPropExprNode */

std::string MenuItemPropExprNode::toString(bool summary) {
    auto propString = Lingo::getName(Lingo::menuItemPropertyNames, prop);
    return "menu(" + menuID->toString(summary) + ").item(" + itemID->toString(summary) + ")." + propString;
}

/* SoundPropExprNode */

std::string SoundPropExprNode::toString(bool summary) {
    auto propString = Lingo::getName(Lingo::soundPropertyNames, prop);
    return "sound(" + soundID->toString(summary) + ")." + propString;
}

/* SpritePropExprNode */

std::string SpritePropExprNode::toString(bool summary) {
    auto propString = Lingo::getName(Lingo::spritePropertyNames, prop);
    return "sprite(" + spriteID->toString(summary) + ")." + propString;
}

/* CastPropExprNode */

std::string CastPropExprNode::toString(bool summary) {
    return "cast(" + castID->toString(summary) + ")." + prop;
}

/* FieldPropExprNode */

std::string FieldPropExprNode::toString(bool summary) {
    auto propString = Lingo::getName(Lingo::fieldPropertyNames, prop);
    return "field(" + fieldID->toString(summary) + ")." + propString;
}

/* ObjPropExprNode */

std::string ObjPropExprNode::toString(bool summary) {
    return obj->toString(summary) + "." + prop;
}

/* ExitRepeatStmtNode */

std::string ExitRepeatStmtNode::toString(bool summary) {
    return "exit repeat";
}

/* NextRepeatStmtNode */

std::string NextRepeatStmtNode::toString(bool summary) {
    return "next repeat";
}

/* PutStmtNode */

std::string PutStmtNode::toString(bool summary) {
    auto typeString = Lingo::getName(Lingo::putTypeNames, type);
    return "put " + value->toString(summary) + " " + typeString + " " + variable->toString(summary);
}

}
