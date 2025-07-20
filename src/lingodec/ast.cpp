/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/codewriter.h"
#include "common/util.h"
#include "lingodec/ast.h"
#include "lingodec/handler.h"
#include "lingodec/names.h"
#include "lingodec/script.h"

namespace LingoDec {

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

void Datum::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	switch (type) {
	case kDatumVoid:
		code.write("VOID");
		return;
	case kDatumSymbol:
		code.write("#" + s);
		return;
	case kDatumVarRef:
		code.write(s);
		return;
	case kDatumString:
		if (s.size() == 0) {
			code.write("EMPTY");
			return;
		}
		if (s.size() == 1) {
			switch (s[0]) {
			case '\x03':
				code.write("ENTER");
				return;
			case '\x08':
				code.write("BACKSPACE");
				return;
			case '\t':
				code.write("TAB");
				return;
			case '\r':
				code.write("RETURN");
				return;
			case '"':
				code.write("QUOTE");
				return;
			default:
				break;
			}
		}
		if (sum) {
			code.write("\"" + Common::escapeString(s) + "\"");
			return;
		}
		code.write("\"" + s + "\"");
		return;
	case kDatumInt:
		code.write(std::to_string(i));
		return;
	case kDatumFloat:
		code.write(Common::floatToString(f));
		return;
	case kDatumList:
	case kDatumArgList:
	case kDatumArgListNoRet:
		{
			if (type == kDatumList)
				code.write("[");
			for (size_t i = 0; i < l.size(); i++) {
				if (i > 0)
					code.write(", ");
				l[i]->writeScriptText(code, dot, sum);
			}
			if (type == kDatumList)
				code.write("]");
		}
		return;
	case kDatumPropList:
		{
			code.write("[");
			if (l.size() == 0) {
				code.write(":");
			} else {
				for (size_t i = 0; i < l.size(); i += 2) {
					if (i > 0)
						code.write(", ");
					l[i]->writeScriptText(code, dot, sum);
					code.write(": ");
					l[i + 1]->writeScriptText(code, dot, sum);
				}
			}
			code.write("]");
		}
		return;
	}
}

/* AST */

void AST::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	root->writeScriptText(code, dot, sum);
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

void ErrorNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("ERROR");
}

bool ErrorNode::hasSpaces(bool) {
	return false;
}

/* CommentNode */

void CommentNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("-- ");
	code.write(text);
}

/* LiteralNode */

void LiteralNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	value->writeScriptText(code, dot, sum);
}

std::shared_ptr<Datum> LiteralNode::getValue() {
	return value;
}

bool LiteralNode::hasSpaces(bool) {
	return false;
}

/* BlockNode */

void BlockNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	for (const auto &child : children) {
		child->writeScriptText(code, dot, sum);
		code.writeLine();
	}
}

void BlockNode::addChild(std::shared_ptr<Node> child) {
	child->parent = this;
	children.push_back(std::move(child));
}

/* HandlerNode */

void HandlerNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	if (handler->isGenericEvent) {
		block->writeScriptText(code, dot, sum);
	} else {
		Script *script = handler->script;
		bool isMethod = script->isFactory();
		if (isMethod) {
			code.write("method ");
		} else {
			code.write("on ");
		}
		code.write(handler->name);
		if (handler->argumentNames.size() > 0) {
			code.write(" ");
			for (size_t i = 0; i < handler->argumentNames.size(); i++) {
				if (i > 0)
					code.write(", ");
				code.write(handler->argumentNames[i]);
			}
		}
		code.writeLine();
		code.indent();
		if (isMethod && script->propertyNames.size() > 0 && handler == script->handlers[0].get()) {
			code.write("instance ");
			for (size_t i = 0; i < script->propertyNames.size(); i++) {
				if (i > 0)
					code.write(", ");
				code.write(script->propertyNames[i]);
			}
			code.writeLine();
		}
		if (handler->globalNames.size() > 0) {
			code.write("global ");
			for (size_t i = 0; i < handler->globalNames.size(); i++) {
				if (i > 0)
					code.write(", ");
				code.write(handler->globalNames[i]);
			}
			code.writeLine();
		}
		block->writeScriptText(code, dot, sum);
		code.unindent();
		if (!isMethod) {
			code.writeLine("end");
		}
	}
}

/* ExitStmtNode */

void ExitStmtNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("exit");
}

/* InverseOpNode */

void InverseOpNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("-");

	bool parenOperand = operand->hasSpaces(dot);
	if (parenOperand) {
		code.write("(");
	}
	operand->writeScriptText(code, dot, sum);
	if (parenOperand) {
		code.write(")");
	}
}

/* NotOpNode */

void NotOpNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("not ");

	bool parenOperand = operand->hasSpaces(dot);
	if (parenOperand) {
		code.write("(");
	}
	operand->writeScriptText(code, dot, sum);
	if (parenOperand) {
		code.write(")");
	}
}

/* BinaryOpNode */

void BinaryOpNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	unsigned int precedence = getPrecedence();
	bool parenLeft = false;
	bool parenRight = false;
	if (precedence) {
		if (left->type == kBinaryOpNode) {
			auto leftBinaryOpNode = static_cast<BinaryOpNode *>(left.get());
			parenLeft = (leftBinaryOpNode->getPrecedence() != precedence);
		}
		parenRight = (right->type == kBinaryOpNode);
	}

	if (parenLeft) {
		code.write("(");
	}
	left->writeScriptText(code, dot, sum);
	if (parenLeft) {
		code.write(")");
	}

	code.write(" ");
	code.write(StandardNames::getName(StandardNames::binaryOpNames, opcode));
	code.write(" ");

	if (parenRight) {
		code.write("(");
	}
	right->writeScriptText(code, dot, sum);
	if (parenRight) {
		code.write(")");
	}
}

unsigned int BinaryOpNode::getPrecedence() const {
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

void ChunkExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write(StandardNames::getName(StandardNames::chunkTypeNames, type));
	code.write(" ");
	bool parenFirst = first->hasSpaces(dot);
	if (parenFirst) {
		code.write("(");
	}
	first->writeScriptText(code, dot, sum);
	if (parenFirst) {
		code.write(")");
	}
	if (!(last->type == kLiteralNode && last->getValue()->type == kDatumInt && last->getValue()->i == 0)) {
		code.write(" to ");
		bool parenLast = last->hasSpaces(dot);
		if (parenLast) {
			code.write("(");
		}
		last->writeScriptText(code, dot, sum);
		if (parenLast) {
			code.write(")");
		}
	}
	code.write(" of ");
	bool stringIsBiggerChunk = string->type == kChunkExprNode && static_cast<ChunkExprNode *>(string.get())->type > this->type;
	bool parenString = !stringIsBiggerChunk && string->hasSpaces(dot);
	if (parenString) {
		code.write("(");
	}
	string->writeScriptText(code, false, sum); // we want the string to always be verbose
	if (parenString) {
		code.write(")");
	}
}

/* ChunkHiliteStmtNode */

void ChunkHiliteStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("hilite ");
	chunk->writeScriptText(code, dot, sum);
}

/* ChunkDeleteStmtNode */

void ChunkDeleteStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("delete ");
	chunk->writeScriptText(code, dot, sum);
}

/* SpriteIntersectsExprNode */

void SpriteIntersectsExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("sprite ");

	bool parenFirstSprite = (firstSprite->type == kBinaryOpNode);
	if (parenFirstSprite) {
		code.write("(");
	}
	firstSprite->writeScriptText(code, dot, sum);
	if (parenFirstSprite) {
		code.write(")");
	}

	code.write(" intersects ");

	bool parenSecondSprite = (secondSprite->type == kBinaryOpNode);
	if (parenSecondSprite) {
		code.write("(");
	}
	secondSprite->writeScriptText(code, dot, sum);
	if (parenSecondSprite) {
		code.write(")");
	}
}

/* SpriteWithinExprNode */

void SpriteWithinExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("sprite ");

	bool parenFirstSprite = (firstSprite->type == kBinaryOpNode);
	if (parenFirstSprite) {
		code.write("(");
	}
	firstSprite->writeScriptText(code, dot, sum);
	if (parenFirstSprite) {
		code.write(")");
	}

	code.write(" within ");

	bool parenSecondSprite = (secondSprite->type == kBinaryOpNode);
	if (parenSecondSprite) {
		code.write("(");
	}
	secondSprite->writeScriptText(code, dot, sum);
	if (parenSecondSprite) {
		code.write(")");
	}
}

/* MemberExprNode */

void MemberExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	bool hasCastID = castID && !(castID->type == kLiteralNode && castID->getValue()->type == kDatumInt && castID->getValue()->i == 0);
	code.write(type);
	if (dot) {
		code.write("(");
		memberID->writeScriptText(code, dot, sum);
		if (hasCastID) {
			code.write(", ");
			castID->writeScriptText(code, dot, sum);
		}
		code.write(")");
	} else {
		code.write(" ");

		bool parenMemberID = (memberID->type == kBinaryOpNode);
		if (parenMemberID) {
			code.write("(");
		}
		memberID->writeScriptText(code, dot, sum);
		if (parenMemberID) {
			code.write(")");
		}

		if (hasCastID) {
			code.write(" of castLib ");

			bool parenCastID = (castID->type == kBinaryOpNode);
			if (parenCastID) {
				code.write("(");
			}
			castID->writeScriptText(code, dot, sum);
			if (parenCastID) {
				code.write(")");
			}
		}
	}
}

bool MemberExprNode::hasSpaces(bool dot) {
	return !dot;
}

/* VarNode */

void VarNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write(varName);
}

bool VarNode::hasSpaces(bool) {
	return false;
}

/* AssignmentStmtNode */

void AssignmentStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	if (!dot || forceVerbose) {
		code.write("set ");
		variable->writeScriptText(code, false, sum); // we want the variable to always be verbose
		code.write(" to ");
		value->writeScriptText(code, dot, sum);
	} else {
		variable->writeScriptText(code, dot, sum);
		code.write(" = ");
		value->writeScriptText(code, dot, sum);
	}
}

/* IfStmtNode */

void IfStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("if ");
	condition->writeScriptText(code, dot, sum);
	code.write(" then");
	if (sum) {
		if (hasElse) {
			code.write(" / else");
		}
	} else {
		code.writeLine();
		code.indent();
		block1->writeScriptText(code, dot, sum);
		code.unindent();
		if (hasElse) {
			code.writeLine("else");
			code.indent();
			block2->writeScriptText(code, dot, sum);
			code.unindent();
		}
		code.write("end if");
	}
}

/* RepeatWhileStmtNode */

void RepeatWhileStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("repeat while ");
	condition->writeScriptText(code, dot, sum);
	if (!sum) {
		code.writeLine();
		code.indent();
		block->writeScriptText(code, dot, sum);
		code.unindent();
		code.write("end repeat");
	}
}

/* RepeatWithInStmtNode */

void RepeatWithInStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("repeat with ");
	code.write(varName);
	code.write(" in ");
	list->writeScriptText(code, dot, sum);
	if (!sum) {
		code.writeLine();
		code.indent();
		block->writeScriptText(code, dot, sum);
		code.unindent();
		code.write("end repeat");
	}
}

/* RepeatWithToStmtNode */

void RepeatWithToStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("repeat with ");
	code.write(varName);
	code.write(" = ");
	start->writeScriptText(code, dot, sum);
	if (up) {
		code.write(" to ");
	} else {
		code.write(" down to ");
	}
	end->writeScriptText(code, dot, sum);
	if (!sum) {
		code.writeLine();
		code.indent();
		block->writeScriptText(code, dot, sum);
		code.unindent();
		code.write("end repeat");
	}
}

/* CaseLabelNode */

void CaseLabelNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	if (sum) {
		code.write("(case) ");
		if (parent->type == kCaseLabelNode) {
			auto parentLabel = static_cast<CaseLabelNode *>(parent);
			if (parentLabel->nextOr.get() == this) {
				code.write("..., ");
			}
		}

		bool parenValue = value->hasSpaces(dot);
		if (parenValue) {
			code.write("(");
		}
		value->writeScriptText(code, dot, sum);
		if (parenValue) {
			code.write(")");
		}

		if (nextOr) {
			code.write(", ...");
		} else {
			code.write(":");
		}
	} else {
		bool parenValue = value->hasSpaces(dot);
		if (parenValue) {
			code.write("(");
		}
		value->writeScriptText(code, dot, sum);
		if (parenValue) {
			code.write(")");
		}

		if (nextOr) {
			code.write(", ");
			nextOr->writeScriptText(code, dot, sum);
		} else {
			code.writeLine(":");
			code.indent();
			block->writeScriptText(code, dot, sum);
			code.unindent();
		}
		if (nextLabel) {
			nextLabel->writeScriptText(code, dot, sum);
		}
	}
}

/* OtherwiseNode */

void OtherwiseNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	if (sum) {
		code.write("(case) otherwise:");
	} else {
		code.writeLine("otherwise:");
		code.indent();
		block->writeScriptText(code, dot, sum);
		code.unindent();
	}
}

/* EndCaseNode */

void EndCaseNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("end case");
}

/* CaseStmtNode */

void CaseStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("case ");
	value->writeScriptText(code, dot, sum);
	code.write(" of");
	if (sum) {
		if (!firstLabel) {
			if (otherwise) {
				code.write(" / otherwise:");
			} else {
				code.write(" / end case");
			}
		}
	} else {
		code.writeLine();
		code.indent();
		if (firstLabel) {
			firstLabel->writeScriptText(code, dot, sum);
		}
		if (otherwise) {
			otherwise->writeScriptText(code, dot, sum);
		}
		code.unindent();
		code.write("end case");
	}
}

void CaseStmtNode::addOtherwise() {
	otherwise = std::make_shared<OtherwiseNode>();
	otherwise->parent = this;
	otherwise->block->endPos = endPos;
}

/* TellStmtNode */

void TellStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("tell ");
	window->writeScriptText(code, dot, sum);
	if (!sum) {
		code.writeLine();
		code.indent();
		block->writeScriptText(code, dot, sum);
		code.unindent();
		code.write("end tell");
	}
}

/* SoundCmdStmtNode */

void SoundCmdStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("sound ");
	code.write(cmd);
	if (argList->getValue()->l.size() > 0) {
		code.write(" ");
		argList->writeScriptText(code, dot, sum);
	}
}

/* PlayCmdStmtNode */

void PlayCmdStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	auto &rawArgs = argList->getValue()->l;

	code.write("play");

	if (rawArgs.size() == 0) {
		code.write(" done");
		return;
	}

	auto &frame = rawArgs[0];
	if (rawArgs.size() == 1) {
		code.write(" frame ");
		frame->writeScriptText(code, dot, sum);
		return;
	}

	auto &movie = rawArgs[1];
	if (!(frame->type == kLiteralNode && frame->getValue()->type == kDatumInt && frame->getValue()->i == 1)) {
		code.write(" frame ");
		frame->writeScriptText(code, dot, sum);
		code.write(" of");
	}
	code.write(" movie ");
	movie->writeScriptText(code, dot, sum);
}

/* CallNode */

bool CallNode::noParens() const {
	if (isStatement) {
		// TODO: Make a complete list of commonly paren-less commands
		if (name == "put")
			return true;
		if (name == "return")
			return true;
	}

	return false;
}

bool CallNode::isMemberExpr() const {
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

void CallNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	if (isExpression && argList->getValue()->l.size() == 0) {
		if (name == "pi") {
			code.write("PI");
			return;
		}
		if (name == "space") {
			code.write("SPACE");
			return;
		}
		if (name == "void") {
			code.write("VOID");
			return;
		}
	}

	if (!dot && isMemberExpr()) {
		/**
		 * In some cases, member expressions such as `member 1 of castLib 1` compile
		 * to the function call `member(1, 1)`. However, this doesn't parse correctly
		 * in pre-dot-syntax versions of Director, and `put(member(1, 1))` does not
		 * compile. Therefore, we rewrite these expressions to the verbose syntax when
		 * in verbose mode.
		 */
		code.write(name);
		code.write(" ");

		auto memberID = argList->getValue()->l[0];
		bool parenMemberID = (memberID->type == kBinaryOpNode);
		if (parenMemberID) {
			code.write("(");
		}
		memberID->writeScriptText(code, dot, sum);
		if (parenMemberID) {
			code.write(")");
		}

		if (argList->getValue()->l.size() == 2) {
			code.write(" of castLib ");

			auto castID = argList->getValue()->l[1];
			bool parenCastID = (castID->type == kBinaryOpNode);
			if (parenCastID) {
				code.write("(");
			}
			castID->writeScriptText(code, dot, sum);
			if (parenCastID) {
				code.write(")");
			}
		}
		return;
	}

	code.write(name);
	if (noParens()) {
		code.write(" ");
		argList->writeScriptText(code, dot, sum);
	} else {
		code.write("(");
		argList->writeScriptText(code, dot, sum);
		code.write(")");
	}
}

bool CallNode::hasSpaces(bool dot) {
	if (!dot && isMemberExpr())
		return true;

	if (noParens())
		return true;

	return false;
}

/* ObjCallNode */

void ObjCallNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	auto &rawArgs = argList->getValue()->l;

	auto &obj = rawArgs[0];
	bool parenObj = obj->hasSpaces(dot);
	if (parenObj) {
		code.write("(");
	}
	obj->writeScriptText(code, dot, sum);
	if (parenObj) {
		code.write(")");
	}

	code.write(".");
	code.write(name);
	code.write("(");
	for (size_t i = 1; i < rawArgs.size(); i++) {
		if (i > 1)
			code.write(", ");
		rawArgs[i]->writeScriptText(code, dot, sum);
	}
	code.write(")");
}

bool ObjCallNode::hasSpaces(bool) {
	return false;
}

/* ObjCallV4Node */

void ObjCallV4Node::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	obj->writeScriptText(code, dot, sum);
	code.write("(");
	argList->writeScriptText(code, dot, sum);
	code.write(")");
}

bool ObjCallV4Node::hasSpaces(bool) {
	return false;
}

/* TheExprNode */

void TheExprNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("the ");
	code.write(prop);
}

/* LastStringChunkExprNode */

void LastStringChunkExprNode::writeScriptText(Common::CodeWriter &code, bool, bool sum) const {
	code.write("the last ");
	code.write(StandardNames::getName(StandardNames::chunkTypeNames, type));
	code.write(" in ");

	bool parenObj = (obj->type == kBinaryOpNode);
	if (parenObj) {
		code.write("(");
	}
	obj->writeScriptText(code, false, sum); // we want the object to always be verbose
	if (parenObj) {
		code.write(")");
	}
}

/* StringChunkCountExprNode */

void StringChunkCountExprNode::writeScriptText(Common::CodeWriter &code, bool, bool sum) const {
	code.write("the number of ");
	code.write(StandardNames::getName(StandardNames::chunkTypeNames, type)); // we want the object to always be verbose
	code.write("s in ");

	bool parenObj = (obj->type == kBinaryOpNode);
	if (parenObj) {
		code.write("(");
	}
	obj->writeScriptText(code, false, sum);
	if (parenObj) {
		code.write(")");
	}
}

/* MenuPropExprNode */

void MenuPropExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("the ");
	code.write(StandardNames::getName(StandardNames::menuPropertyNames, prop));
	code.write(" of menu ");

	bool parenMenuID = (menuID->type == kBinaryOpNode);
	if (parenMenuID) {
		code.write("(");
	}
	menuID->writeScriptText(code, dot, sum);
	if (parenMenuID) {
		code.write(")");
	}
}

/* MenuItemPropExprNode */

void MenuItemPropExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("the ");
	code.write(StandardNames::getName(StandardNames::menuItemPropertyNames, prop));
	code.write(" of menuItem ");

	bool parenItemID = (itemID->type == kBinaryOpNode);
	if (parenItemID) {
		code.write("(");
	}
	itemID->writeScriptText(code, dot, sum);
	if (parenItemID) {
		code.write(")");
	}

	code.write(" of menu ");

	bool parenMenuID = (menuID->type == kBinaryOpNode);
	if (parenMenuID) {
		code.write("(");
	}
	menuID->writeScriptText(code, dot, sum);
	if (parenMenuID) {
		code.write(")");
	}
}

/* SoundPropExprNode */

void SoundPropExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("the ");
	code.write(StandardNames::getName(StandardNames::soundPropertyNames, prop));
	code.write(" of sound ");

	bool parenSoundID = (soundID->type == kBinaryOpNode);
	if (parenSoundID) {
		code.write("(");
	}
	soundID->writeScriptText(code, dot, sum);
	if (parenSoundID) {
		code.write(")");
	}
}

/* SpritePropExprNode */

void SpritePropExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("the ");
	code.write(StandardNames::getName(StandardNames::spritePropertyNames, prop));
	code.write(" of sprite ");

	bool parenSpriteID = (spriteID->type == kBinaryOpNode);
	if (parenSpriteID) {
		code.write("(");
	}
	spriteID->writeScriptText(code, dot, sum);
	if (parenSpriteID) {
		code.write(")");
	}
}

/* ThePropExprNode */

void ThePropExprNode::writeScriptText(Common::CodeWriter &code, bool, bool sum) const {
	code.write("the ");
	code.write(prop);
	code.write(" of ");

	bool parenObj = (obj->type == kBinaryOpNode);
	if (parenObj) {
		code.write("(");
	}
	obj->writeScriptText(code, false, sum); // we want the object to always be verbose
	if (parenObj) {
		code.write(")");
	}
}

/* ObjPropExprNode */

void ObjPropExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	if (dot) {
		bool parenObj = obj->hasSpaces(dot);
		if (parenObj) {
			code.write("(");
		}
		obj->writeScriptText(code, dot, sum);
		if (parenObj) {
			code.write(")");
		}

		code.write(".");
		code.write(prop);
	} else {
		code.write("the ");
		code.write(prop);
		code.write(" of ");

		bool parenObj = (obj->type == kBinaryOpNode);
		if (parenObj) {
			code.write("(");
		}
		obj->writeScriptText(code, dot, sum);
		if (parenObj) {
			code.write(")");
		}
	}
}

bool ObjPropExprNode::hasSpaces(bool dot) {
	return !dot;
}

/* ObjBracketExprNode */

void ObjBracketExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	bool parenObj = obj->hasSpaces(dot);
	if (parenObj) {
		code.write("(");
	}
	obj->writeScriptText(code, dot, sum);
	if (parenObj) {
		code.write(")");
	}

	code.write("[");
	prop->writeScriptText(code, dot, sum);
	code.write("]");
}

bool ObjBracketExprNode::hasSpaces(bool) {
	return false;
}

/* ObjPropIndexExprNode */

void ObjPropIndexExprNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	bool parenObj = obj->hasSpaces(dot);
	if (parenObj) {
		code.write("(");
	}
	obj->writeScriptText(code, dot, sum);
	if (parenObj) {
		code.write(")");
	}

	code.write(".");
	code.write(prop);
	code.write("[");
	index->writeScriptText(code, dot, sum);
	if (index2) {
		code.write("..");
		index2->writeScriptText(code, dot, sum);
	}
	code.write("]");
}

bool ObjPropIndexExprNode::hasSpaces(bool) {
	return false;
}

/* ExitRepeatStmtNode */

void ExitRepeatStmtNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("exit repeat");
}

/* NextRepeatStmtNode */

void NextRepeatStmtNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("next repeat");
}

/* PutStmtNode */

void PutStmtNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("put ");
	value->writeScriptText(code, dot, sum);
	code.write(" ");
	code.write(StandardNames::getName(StandardNames::putTypeNames, type));
	code.write(" ");
	variable->writeScriptText(code, false, sum); // we want the variable to always be verbose
}

/* WhenStmtNode */

void WhenStmtNode::writeScriptText(Common::CodeWriter &code, bool, bool) const {
	code.write("when ");
	code.write(StandardNames::getName(StandardNames::whenEventNames, event));
	code.write(" then");

	code.doIndentation = false;
	for (size_t i = 0; i < script.size(); i++) {
		char ch = script[i];
		if (ch == '\r') {
			if (i != script.size() - 1) {
				code.writeLine();
			}
		} else {
			code.write(ch);
		}
	}
	code.doIndentation = true;
}

/* NewObjNode */

void NewObjNode::writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const {
	code.write("new ");
	code.write(objType);
	code.write("(");
	objArgs->writeScriptText(code, dot, sum);
	code.write(")");
}

} // namespace LingoDec
