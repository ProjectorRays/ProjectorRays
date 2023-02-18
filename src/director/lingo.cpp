/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/json.h"
#include "common/util.h"
#include "director/lingo.h"
#include "director/util.h"

namespace Director {

/* Lingo */

std::map<unsigned int, std::string> Lingo::opcodeNames = {
	// single-byte
	{ kOpRet,				"ret" },
	{ kOpRetFactory,		"retfactory" },
	{ kOpPushZero,			"pushzero" },
	{ kOpMul,				"mul" },
	{ kOpAdd,				"add" },
	{ kOpSub,				"sub" },
	{ kOpDiv,				"div" },
	{ kOpMod,				"mod" },
	{ kOpInv,				"inv" },
	{ kOpJoinStr,			"joinstr" },
	{ kOpJoinPadStr,		"joinpadstr" },
	{ kOpLt,				"lt" },
	{ kOpLtEq,				"lteq" },
	{ kOpNtEq,				"nteq" },
	{ kOpEq,				"eq" },
	{ kOpGt,				"gt" },
	{ kOpGtEq,				"gteq" },
	{ kOpAnd,				"and" },
	{ kOpOr,				"or" },
	{ kOpNot,				"not" },
	{ kOpContainsStr,		"containsstr" },
	{ kOpContains0Str,		"contains0str" },
	{ kOpGetChunk,			"getchunk" },
	{ kOpHiliteChunk,		"hilitechunk" },
	{ kOpOntoSpr,			"ontospr" },
	{ kOpIntoSpr,			"intospr" },
	{ kOpGetField,			"getfield" },
	{ kOpStartTell,			"starttell" },
	{ kOpEndTell,			"endtell" },
	{ kOpPushList,			"pushlist" },
	{ kOpPushPropList,		"pushproplist" },
	{ kOpSwap,				"swap" },

	// multi-byte
	{ kOpPushInt8,			"pushint8" },
	{ kOpPushArgListNoRet,	"pusharglistnoret" },
	{ kOpPushArgList,		"pusharglist" },
	{ kOpPushCons,			"pushcons" },
	{ kOpPushSymb,			"pushsymb" },
	{ kOpPushVarRef,		"pushvarref" },
	{ kOpGetGlobal2,		"getglobal2" },
	{ kOpGetGlobal,			"getglobal" },
	{ kOpGetProp,			"getprop" },
	{ kOpGetParam,			"getparam" },
	{ kOpGetLocal,			"getlocal" },
	{ kOpSetGlobal2,		"setglobal2" },
	{ kOpSetGlobal,			"setglobal" },
	{ kOpSetProp,			"setprop" },
	{ kOpSetParam,			"setparam" },
	{ kOpSetLocal,			"setlocal" },
	{ kOpJmp,				"jmp" },
	{ kOpEndRepeat,			"endrepeat" },
	{ kOpJmpIfZ,			"jmpifz" },
	{ kOpLocalCall,			"localcall" },
	{ kOpExtCall,			"extcall" },
	{ kOpObjCallV4,			"objcallv4" },
	{ kOpPut,				"put" },
	{ kOpPutChunk,			"putchunk" },
	{ kOpDeleteChunk,		"deletechunk" },
	{ kOpGet,				"get" },
	{ kOpSet,				"set" },
	{ kOpGetMovieProp,		"getmovieprop" },
	{ kOpSetMovieProp,		"setmovieprop" },
	{ kOpGetObjProp,		"getobjprop" },
	{ kOpSetObjProp,		"setobjprop" },
	{ kOpTellCall,			"tellcall" },
	{ kOpPeek,				"peek" },
	{ kOpPop,				"pop" },
	{ kOpTheBuiltin,		"thebuiltin" },
	{ kOpObjCall,			"objcall" },
	{ kOpPushChunkVarRef,	"pushchunkvarref" },
	{ kOpPushInt16,			"pushint16" },
	{ kOpPushInt32,			"pushint32" },
	{ kOpGetChainedProp,	"getchainedprop" },
	{ kOpPushFloat32,		"pushfloat32" },
	{ kOpGetTopLevelProp,	"gettoplevelprop" },
	{ kOpNewObj,			"newobj" }
};

std::map<unsigned int, std::string> Lingo::binaryOpNames = {
	{ kOpMul,			"*" },
	{ kOpAdd,			"+" },
	{ kOpSub,			"-" },
	{ kOpDiv,			"/" },
	{ kOpMod,			"mod" },
	{ kOpJoinStr,		"&" },
	{ kOpJoinPadStr,	"&&" },
	{ kOpLt,			"<" },
	{ kOpLtEq,			"<=" },
	{ kOpNtEq,			"<>" },
	{ kOpEq,			"=" },
	{ kOpGt,			">" },
	{ kOpGtEq,			">=" },
	{ kOpAnd,			"and" },
	{ kOpOr,			"or" },
	{ kOpContainsStr,	"contains" },
	{ kOpContains0Str,	"starts" }
};

std::map<unsigned int, std::string> Lingo::chunkTypeNames = {
	{ kChunkChar, "char" },
	{ kChunkWord, "word" },
	{ kChunkItem, "item" },
	{ kChunkLine, "line" }
};

std::map<unsigned int, std::string> Lingo::putTypeNames = {
	{ kPutInto,		"into" },
	{ kPutAfter,	"after" },
	{ kPutBefore,	"before" }
};

std::map<unsigned int, std::string> Lingo::moviePropertyNames = {
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

std::map<unsigned int, std::string> Lingo::whenEventNames = {
	{ 0x01, "mouseDown" },
	{ 0x02, "mouseUp" },
	{ 0x03, "keyDown" },
	{ 0x04, "keyUp" },
	{ 0x05, "timeOut" },
};

std::map<unsigned int, std::string> Lingo::menuPropertyNames = {
	{ 0x01, "name" },
	{ 0x02, "number of menuItems" }
};

std::map<unsigned int, std::string> Lingo::menuItemPropertyNames = {
	{ 0x01, "name" },
	{ 0x02, "checkMark" },
	{ 0x03, "enabled" },
	{ 0x04, "script" }
};

std::map<unsigned int, std::string> Lingo::soundPropertyNames = {
	{ 0x01, "volume" }
};

std::map<unsigned int, std::string> Lingo::spritePropertyNames = {
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
	{ 0x22, "rect" },
	{ 0x23, "memberNum" },
	{ 0x24, "castLibNum" },
	{ 0x25, "member" },
	{ 0x26, "scriptInstanceList" },
	{ 0x27, "currentTime" },
	{ 0x28, "mostRecentCuePoint" },
	{ 0x29, "tweened" },
	{ 0x2a, "name" }
};

std::map<unsigned int, std::string> Lingo::animationPropertyNames = {
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
	{ 0x0d, "key" },
	{ 0x0e, "lastClick" },
	{ 0x0f, "lastEvent" },
	{ 0x10, "keyCode" },
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
	// 0x1c indicates dontPassEvent was called.
	// It doesn't seem to have a Lingo-accessible name.
	{ 0x1d, "switchColorDepth" },
	{ 0x1e, "timeoutKeyDown" },
	{ 0x1f, "timeoutLength" },
	{ 0x20, "timeoutMouse" },
	{ 0x21, "timeoutPlay" },
	{ 0x22, "timer" },
	{ 0x23, "preLoadRAM" },
	{ 0x24, "videoForWindowsPresent" },
	{ 0x25, "netPresent" },
	{ 0x26, "safePlayer" },
	{ 0x27, "soundKeepDevice" },
	{ 0x28, "soundMixMedia" }
};

std::map<unsigned int, std::string> Lingo::animation2PropertyNames = {
	{ 0x01, "perFrameHook" },
	{ 0x02, "number of castMembers" },
	{ 0x03, "number of menus" },
	{ 0x04, "number of castLibs" },
	{ 0x05, "number of xtras" }
};

std::map<unsigned int, std::string> Lingo::memberPropertyNames = {
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
	{ 0x12, "backColor" },
	{ 0x13, "type" }
};

std::string Lingo::getOpcodeName(uint8_t id) {
	if (id >= 0x40)
		id = 0x40 + id % 0x40;
	auto it = opcodeNames.find(id);
	if (it == opcodeNames.end()){
		return "unk" + byteToString(id);
	}
	return it->second;
}

std::string Lingo::getName(const std::map<unsigned int, std::string> &nameMap, unsigned int id) {
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
		if (s.size() == 0)
			return "EMPTY";
		if (s.size() == 1) {
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

void Datum::writeJSON(Common::JSONWriter &json) const {
	switch (type) {
	case kDatumString:
		json.writeVal(s);
		break;
	case kDatumInt:
		json.writeVal(i);
		break;
	case kDatumFloat:
		json.writeVal(f);
		break;
	default:
		json.writeNull();
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

std::string Node::toString(bool, bool) {
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

bool Node::hasSpaces(bool) {
	return true;
}

/* ErrorNode */

std::string ErrorNode::toString(bool, bool) {
	return "ERROR";
}

bool ErrorNode::hasSpaces(bool) {
	return false;
}

/* CommentNode */

std::string CommentNode::toString(bool, bool) {
	return "-- " + text;
}

/* LiteralNode */

std::string LiteralNode::toString(bool dot, bool sum) {
	return value->toString(dot, sum);
}

std::shared_ptr<Datum> LiteralNode::getValue() {
	return value;
}

bool LiteralNode::hasSpaces(bool) {
	return false;
}

/* BlockNode */

std::string BlockNode::toString(bool dot, bool sum) {
	std::string res = "";
	for (const auto &child : children) {
		res += indent(child->toString(dot, sum) + kLingoLineEnding);
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
	res += kLingoLineEnding;
	if (handler->globalNames.size() > 0) {
		res += "  global ";
		for (size_t i = 0; i < handler->globalNames.size(); i++) {
			if (i > 0)
				res += ", ";
			res += handler->globalNames[i];
		}
		res += kLingoLineEnding;
	}
	res += block->toString(dot, sum);
	res += "end";
	res += kLingoLineEnding;
	return res;
}

/* ExitStmtNode */

std::string ExitStmtNode::toString(bool, bool) {
	return "exit";
}

/* InverseOpNode */

std::string InverseOpNode::toString(bool dot, bool sum) {
	std::string operandString = operand->toString(dot, sum);
	if (operand->hasSpaces(dot)) {
		operandString = "(" + operandString + ")";
	}
	return "-" + operandString;
}

/* NotOpNode */

std::string NotOpNode::toString(bool dot, bool sum) {
	std::string operandString = operand->toString(dot, sum);
	if (operand->hasSpaces(dot)) {
		operandString = "(" + operandString + ")";
	}
	return "not " + operandString;
}

/* BinaryOpNode */

std::string BinaryOpNode::toString(bool dot, bool sum) {
	auto opString = Lingo::getName(Lingo::binaryOpNames, opcode);
	std::string leftString = left->toString(dot, sum);
	std::string rightString = right->toString(dot, sum);
	unsigned int precedence = getPrecedence();
	if (precedence) {
		if (left->type == kBinaryOpNode) {
			auto leftBinaryOpNode = static_cast<BinaryOpNode *>(left.get());
			if (leftBinaryOpNode->getPrecedence() != precedence)
				leftString = "(" + leftString + ")";
		}
		if (right->type == kBinaryOpNode) {
			rightString = "(" + rightString + ")";
		}
	}
	return leftString + " " +  opString + " " + rightString;
}

unsigned int BinaryOpNode::getPrecedence() {
	switch (opcode) {
	case kOpMul:
	case kOpDiv:
	case kOpMod:
		return 1;
	case kOpAdd:
	case kOpSub:
		return 2;
	case kOpLt:
	case kOpLtEq:
	case kOpNtEq:
	case kOpEq:
	case kOpGt:
	case kOpGtEq:
		return 3;
	case kOpAnd:
		return 4;
	case kOpOr:
		return 5;
	default:
		break;
	}
	return 0;
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
	std::string firstSpriteString = firstSprite->toString(dot, sum);
	if (firstSprite->type == kBinaryOpNode) {
		firstSpriteString = "(" + firstSpriteString + ")";
	}
	std::string secondSpriteString = secondSprite->toString(dot, sum);
	if (secondSprite->type == kBinaryOpNode) {
		secondSpriteString = "(" + secondSpriteString + ")";
	}
	return "sprite " + firstSpriteString + " intersects " + secondSpriteString;
}

/* SpriteWithinExprNode */

std::string SpriteWithinExprNode::toString(bool dot, bool sum) {
	std::string firstSpriteString = firstSprite->toString(dot, sum);
	if (firstSprite->type == kBinaryOpNode) {
		firstSpriteString = "(" + firstSpriteString + ")";
	}
	std::string secondSpriteString = secondSprite->toString(dot, sum);
	if (secondSprite->type == kBinaryOpNode) {
		secondSpriteString = "(" + secondSpriteString + ")";
	}
	return "sprite " + firstSpriteString + " within " + secondSpriteString;
}

/* MemberExprNode */

std::string MemberExprNode::toString(bool dot, bool sum) {
	bool hasCastID = castID && !(castID->type == kLiteralNode && castID->getValue()->type == kDatumInt && castID->getValue()->i == 0);
	std::string res = type;
	if (dot) {
		res += "(" + memberID->toString(dot, sum);
		if (hasCastID) {
			res += ", " + castID->toString(dot, sum) + ")";
		} else {
			res += ")";
		}
	} else {
		std::string memberIDString = memberID->toString(dot, sum);
		if (memberID->type == kBinaryOpNode) {
			memberIDString = "(" + memberIDString + ")";
		}
		res += " " + memberIDString;
		if (hasCastID) {
			std::string castIDString = castID->toString(dot, sum);
			if (castID->type == kBinaryOpNode) {
				castIDString = "(" + castIDString + ")";
			}
			res += " of castLib " + castIDString;
		}
	}
	return res;
}

bool MemberExprNode::hasSpaces(bool dot) {
	return !dot;
}

/* VarNode */

std::string VarNode::toString(bool, bool) {
	return varName;
}

bool VarNode::hasSpaces(bool) {
	return false;
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
		res += kLingoLineEnding;
		res += block1->toString(dot, sum);
		if (hasElse) {
			res += "else";
			res += kLingoLineEnding;
			res += block2->toString(dot, sum);
		}
		res += "end if";
	}
	return res;
}

/* RepeatWhileStmtNode */

std::string RepeatWhileStmtNode::toString(bool dot, bool sum) {
	std::string res = "repeat while " + condition->toString(dot, sum);
	if (!sum) {
		res += kLingoLineEnding;
		res += block->toString(dot, sum);
		res += "end repeat";
	}
	return res;
}

/* RepeatWithInStmtNode */

std::string RepeatWithInStmtNode::toString(bool dot, bool sum) {
	std::string res = "repeat with " + varName + " in " + list->toString(dot, sum);
	if (!sum) {
		res += kLingoLineEnding;
		res += block->toString(dot, sum);
		res += "end repeat";
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
		res += kLingoLineEnding;
		res += block->toString(dot, sum);
		res += "end repeat";
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
		std::string valueString = value->toString(dot, sum);
		if (value->hasSpaces(dot)) {
			valueString = "(" + valueString + ")";
		}
		res += valueString;
		if (nextOr) {
			res += ", ...";
		} else {
			res += ":";
		}
	} else {
		std::string valueString = value->toString(dot, sum);
		if (value->hasSpaces(dot)) {
			valueString = "(" + valueString + ")";
		}
		res += valueString;
		if (nextOr) {
			res += ", " + nextOr->toString(dot, sum);
		} else {
			res += ":";
			res += kLingoLineEnding;
			res += block->toString(dot, sum);
		}
		if (nextCase) {
			res += nextCase->toString(dot, sum);
		} else if (otherwise) {
			res += "otherwise:";
			res += kLingoLineEnding;
			res += otherwise->toString(dot, sum);
		}
	}
	return res;
}

/* CasesStmtNode */

std::string CasesStmtNode::toString(bool dot, bool sum) {
	std::string res = "case " + value->toString(dot, sum) + " of";
	if (!sum) {
		res += kLingoLineEnding;
		res += indent(firstCase->toString(dot, sum));
		res += "end case";
	}
	return res;
}

/* TellStmtNode */

std::string TellStmtNode::toString(bool dot, bool sum) {
	std::string res = "tell " + window->toString(dot, sum);
	if (!sum) {
		res += kLingoLineEnding;
		res += block->toString(dot, sum);
		res += "end tell";
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

bool CallNode::isMemberExpr() {
	if (isExpression) {
		size_t nargs = argList->getValue()->l.size();
		if (name == "cast" && (nargs == 1 || nargs == 2))
			return true;
		if (name == "member" && (nargs == 1 || nargs == 2))
			return true;
		if (name == "script" && (nargs == 1 || nargs == 2))
			return true;
		if (name == "castLib" && nargs == 1)
			return true;
		if (name == "window" && nargs == 1)
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

	if (!dot && isMemberExpr()) {
		/**
		 * In some cases, member expressions such as `member 1 of castLib 1` compile
		 * to the function call `member(1, 1)`. However, this doesn't parse correctly
		 * in pre-dot-syntax versions of Director, and `put(member(1, 1))` does not
		 * compile. Therefore, we rewrite these expressions to the verbose syntax when
		 * in verbose mode.
		 */
		auto memberID = argList->getValue()->l[0];
		std::string memberIDString = memberID->toString(dot, sum);
		if (memberID->type == kBinaryOpNode) {
			memberIDString = "(" + memberIDString + ")";
		}
		std::string res = name + " " + memberIDString;
		if (argList->getValue()->l.size() == 2) {
			auto castID = argList->getValue()->l[1];
			std::string castIDString = castID->toString(dot, sum);
			if (castID->type == kBinaryOpNode) {
				castIDString = "(" + castIDString + ")";
			}
			res += " of castLib " + castIDString;
		}
		return res;
	}

	if (noParens())
		return name + " " + argList->toString(dot, sum);

	return name + "(" + argList->toString(dot, sum) + ")";
}

bool CallNode::hasSpaces(bool dot) {
	if (!dot && isMemberExpr())
		return true;

	if (noParens())
		return true;

	return false;
}

/* ObjCallNode */

std::string ObjCallNode::toString(bool dot, bool sum) {
	auto rawArgs = argList->getValue()->l;
	auto obj = rawArgs[0];
	std::string objString = obj->toString(dot, sum);
	if (obj->hasSpaces(dot)) {
		objString = "(" + objString + ")";
	}
	std::string res = objString + "." + name + "(";
	for (size_t i = 1; i < rawArgs.size(); i++) {
		if (i > 1)
			res += ", ";
		res += rawArgs[i]->toString(dot, sum);
	}
	res += ")";
	return res;
}

bool ObjCallNode::hasSpaces(bool) {
	return false;
}

/* ObjCallV4Node */

std::string ObjCallV4Node::toString(bool dot, bool sum) {
	return obj->toString(dot, sum) + "(" + argList->toString(dot, sum) + ")";
}

bool ObjCallV4Node::hasSpaces(bool) {
	return false;
}

/* TheExprNode */

std::string TheExprNode::toString(bool, bool) {
	return "the " + prop;
}

/* LastStringChunkExprNode */

std::string LastStringChunkExprNode::toString(bool, bool sum) {
	auto typeString = Lingo::getName(Lingo::chunkTypeNames, type);
	// we want the string to always be verbose
	return "the last " + typeString + " in " + string->toString(false, sum);
}

/* StringChunkCountExprNode */

std::string StringChunkCountExprNode::toString(bool, bool sum) {
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

std::string ThePropExprNode::toString(bool, bool sum) {
	// we want the object to always be verbose
	return "the " + prop + " of " + obj->toString(false, sum);
}

/* ObjPropExprNode */

std::string ObjPropExprNode::toString(bool dot, bool sum) {
	if (dot) {
		std::string objString = obj->toString(dot, sum);
		if (obj->hasSpaces(dot)) {
			objString = "(" + objString + ")";
		}
		return objString + "." + prop;
	}
	return "the " + prop + " of " + obj->toString(dot, sum);
}

bool ObjPropExprNode::hasSpaces(bool dot) {
	return !dot;
}

/* ObjBracketExprNode */

std::string ObjBracketExprNode::toString(bool dot, bool sum) {
	std::string objString = obj->toString(dot, sum);
	if (obj->hasSpaces(dot)) {
		objString = "(" + objString + ")";
	}
	return objString + "[" + prop->toString(dot, sum) + "]";
}

bool ObjBracketExprNode::hasSpaces(bool) {
	return false;
}

/* ObjPropIndexExprNode */

std::string ObjPropIndexExprNode::toString(bool dot, bool sum) {
	std::string objString = obj->toString(dot, sum);
	if (obj->hasSpaces(dot)) {
		objString = "(" + objString + ")";
	}
	std::string res = objString + "." + prop + "[" + index->toString(dot, sum);
	if (index2)
		res += ".." + index2->toString(dot, sum);
	res += "]";
	return res;
}

bool ObjPropIndexExprNode::hasSpaces(bool) {
	return false;
}

/* ExitRepeatStmtNode */

std::string ExitRepeatStmtNode::toString(bool, bool) {
	return "exit repeat";
}

/* NextRepeatStmtNode */

std::string NextRepeatStmtNode::toString(bool, bool) {
	return "next repeat";
}

/* PutStmtNode */

std::string PutStmtNode::toString(bool dot, bool sum) {
	auto typeString = Lingo::getName(Lingo::putTypeNames, type);
	// we want the variable to always be verbose
	return "put " + value->toString(dot, sum) + " " + typeString + " " + variable->toString(false, sum);
}

/* WhenStmtNode */

std::string WhenStmtNode::toString(bool, bool) {
	std::string eventName = Lingo::getName(Lingo::whenEventNames, event);
	std::string res = "when " + eventName + " then ";

	// Reformat the script to conform to our auto-indentation...

	size_t i = 0;
	while (true) {
		// Skip spaces.
		while (i < script.size() && isspace(script[i]) && script[i] != kLingoLineEnding) {
			i++;
		}
		if (i == script.size())
			break;

		// Copy script until the end of the line.
		while (i < script.size() && script[i] != kLingoLineEnding) {
			res += script[i];
			i++;
		}
		if (i == script.size())
			break;

		// If there's more script, add a line break and indent.
		if (i < script.size() - 1) {
			res += kLingoLineEnding;
			res += "  ";
		}
		i++;
		if (i == script.size())
			break;
	}

	return res;
}

/* NewObjNode */

std::string NewObjNode::toString(bool dot, bool sum) {
	return "new " + objType + "(" + objArgs->toString(dot, sum) + ")";
}

} // namespace Director
