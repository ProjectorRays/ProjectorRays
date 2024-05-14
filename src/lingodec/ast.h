/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_AST_H
#define LINGODEC_AST_H

#include <memory>

#include "common/array.h"
#include "common/str.h"
#include "lingodec/enums.h"

namespace LingoDec {

struct CaseLabelNode;
struct Handler;
struct LoopNode;
struct Node;
struct RepeatWithInStmtNode;

/* Datum */

struct Datum {
	DatumType type;
	int i;
	double f;
	Common::String s;
	Common::Array<std::shared_ptr<Node>> l;

	Datum() {
		type = kDatumVoid;
	}
	Datum(int val) {
		type = kDatumInt;
		i = val;
	}
	Datum(double val) {
		type = kDatumFloat;
		f = val;
	}
	Datum(DatumType t, Common::String val) {
		type = t;
		s = val;
	}
	Datum(DatumType t, Common::Array<std::shared_ptr<Node>> val) {
		type = t;
		l = val;
	}

	int toInt();
	void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* Node */

struct Node {
	NodeType type;
	bool isExpression;
	bool isStatement;
	bool isLabel;
	bool isLoop;
	Node *parent;

	Node(NodeType t) : type(t), isExpression(false), isStatement(false), isLabel(false), isLoop(false), parent(nullptr) {}
	virtual ~Node() = default;
	virtual void writeScriptText(Common::CodeWriter&, bool, bool) const {}
	virtual std::shared_ptr<Datum> getValue();
	Node *ancestorStatement();
	LoopNode *ancestorLoop();
	virtual bool hasSpaces(bool dot);
};

/* ExprNode */

struct ExprNode : Node {
	ExprNode(NodeType t) : Node(t) {
		isExpression = true;
	}
	virtual ~ExprNode() = default;
};

/* StmtNode */

struct StmtNode : Node {
	StmtNode(NodeType t) : Node(t) {
		isStatement = true;
	}
	virtual ~StmtNode() = default;
};

/* LabelNode */

struct LabelNode : Node {
	LabelNode(NodeType t) : Node(t) {
		isLabel = true;
	}
	virtual ~LabelNode() = default;
};

/* LoopNode */

struct LoopNode : StmtNode {
	uint32_t startIndex;

	LoopNode(NodeType t, uint32_t startIndex) : StmtNode(t), startIndex(startIndex) {
		isLoop = true;
	}
	virtual ~LoopNode() = default;
};

/* ErrorNode */

struct ErrorNode : ExprNode {
	ErrorNode() : ExprNode(kErrorNode) {}
	virtual ~ErrorNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* CommentNode */

struct CommentNode : Node {
	Common::String text;

	CommentNode(Common::String t) : Node(kCommentNode), text(t) {}
	virtual ~CommentNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* LiteralNode */

struct LiteralNode : ExprNode {
	std::shared_ptr<Datum> value;

	LiteralNode(std::shared_ptr<Datum> d) : ExprNode(kLiteralNode) {
		value = std::move(d);
	}
	virtual ~LiteralNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual std::shared_ptr<Datum> getValue();
	virtual bool hasSpaces(bool dot);
};

/* BlockNode */

struct BlockNode : Node {
	Common::Array<std::shared_ptr<Node>> children;

	// for use during translation:
	uint32_t endPos;
	CaseLabelNode *currentCaseLabel;

	BlockNode() : Node(kBlockNode), endPos(-1), currentCaseLabel(nullptr) {}
	virtual ~BlockNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	void addChild(std::shared_ptr<Node> child);
};

/* HandlerNode */

struct HandlerNode : Node {
	Handler *handler;
	std::shared_ptr<BlockNode> block;

	HandlerNode(Handler *h)
		: Node(kHandlerNode), handler(h) {
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~HandlerNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* ExitStmtNode */

struct ExitStmtNode : StmtNode {
	ExitStmtNode() : StmtNode(kExitStmtNode) {}
	virtual ~ExitStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* InverseOpNode */

struct InverseOpNode : ExprNode {
	std::shared_ptr<Node> operand;

	InverseOpNode(std::shared_ptr<Node> o) : ExprNode(kInverseOpNode) {
		operand = std::move(o);
		operand->parent = this;
	}
	virtual ~InverseOpNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* NotOpNode */

struct NotOpNode : ExprNode {
	std::shared_ptr<Node> operand;

	NotOpNode(std::shared_ptr<Node> o) : ExprNode(kNotOpNode) {
		operand = std::move(o);
		operand->parent = this;
	}
	virtual ~NotOpNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* BinaryOpNode */

struct BinaryOpNode : ExprNode {
	OpCode opcode;
	std::shared_ptr<Node> left;
	std::shared_ptr<Node> right;

	BinaryOpNode(OpCode op, std::shared_ptr<Node> a, std::shared_ptr<Node> b)
		: ExprNode(kBinaryOpNode), opcode(op) {
		left = std::move(a);
		left->parent = this;
		right = std::move(b);
		right->parent = this;
	}
	virtual ~BinaryOpNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual unsigned int getPrecedence() const;
};

/* ChunkExprNode */

struct ChunkExprNode : ExprNode {
	ChunkExprType type;
	std::shared_ptr<Node> first;
	std::shared_ptr<Node> last;
	std::shared_ptr<Node> string;

	ChunkExprNode(ChunkExprType t, std::shared_ptr<Node> a, std::shared_ptr<Node> b, std::shared_ptr<Node> s)
		: ExprNode(kChunkExprNode), type(t) {
		first = std::move(a);
		first->parent = this;
		last = std::move(b);
		last->parent = this;
		string = std::move(s);
		string->parent = this;
	}
	virtual ~ChunkExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* ChunkHiliteStmtNode */

struct ChunkHiliteStmtNode : StmtNode {
	std::shared_ptr<Node> chunk;

	ChunkHiliteStmtNode(std::shared_ptr<Node> c) : StmtNode(kChunkHiliteStmtNode) {
		chunk = std::move(c);
		chunk->parent = this;
	}
	virtual ~ChunkHiliteStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* ChunkDeleteStmtNode */

struct ChunkDeleteStmtNode : StmtNode {
	std::shared_ptr<Node> chunk;

	ChunkDeleteStmtNode(std::shared_ptr<Node> c) : StmtNode(kChunkDeleteStmtNode) {
		chunk = std::move(c);
		chunk->parent = this;
	}
	virtual ~ChunkDeleteStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* SpriteIntersectsExprNode */

struct SpriteIntersectsExprNode : ExprNode {
	std::shared_ptr<Node> firstSprite;
	std::shared_ptr<Node> secondSprite;

	SpriteIntersectsExprNode(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
		: ExprNode(kSpriteIntersectsExprNode) {
		firstSprite = std::move(a);
		firstSprite->parent = this;
		secondSprite = std::move(b);
		secondSprite->parent = this;
	}
	virtual ~SpriteIntersectsExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* SpriteWithinExprNode */

struct SpriteWithinExprNode : ExprNode {
	std::shared_ptr<Node> firstSprite;
	std::shared_ptr<Node> secondSprite;

	SpriteWithinExprNode(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
		: ExprNode(kSpriteWithinExprNode) {
		firstSprite = std::move(a);
		firstSprite->parent = this;
		secondSprite = std::move(b);
		secondSprite->parent = this;
	}
	virtual ~SpriteWithinExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* MemberExprNode */

struct MemberExprNode : ExprNode {
	Common::String type;
	std::shared_ptr<Node> memberID;
	std::shared_ptr<Node> castID;

	MemberExprNode(Common::String type, std::shared_ptr<Node> memberID, std::shared_ptr<Node> castID)
		: ExprNode(kMemberExprNode), type(type) {
		this->memberID = std::move(memberID);
		this->memberID->parent = this;
		if (castID) {
			this->castID = std::move(castID);
			this->castID->parent = this;
		}
	}
	virtual ~MemberExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* VarNode */

struct VarNode : ExprNode {
	Common::String varName;

	VarNode(Common::String v) : ExprNode(kVarNode), varName(v) {}
	virtual ~VarNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* AssignmentStmtNode */

struct AssignmentStmtNode : StmtNode {
	std::shared_ptr<Node> variable;
	std::shared_ptr<Node> value;
	bool forceVerbose;

	AssignmentStmtNode(std::shared_ptr<Node> var, std::shared_ptr<Node> val, bool forceVerbose = false)
		: StmtNode(kAssignmentStmtNode), forceVerbose(forceVerbose) {
		variable = std::move(var);
		variable->parent = this;
		value = std::move(val);
		value->parent = this;
	}

	virtual ~AssignmentStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* IfStmtNode */

struct IfStmtNode : StmtNode {
	bool hasElse;
	std::shared_ptr<Node> condition;
	std::shared_ptr<BlockNode> block1;
	std::shared_ptr<BlockNode> block2;

	IfStmtNode(std::shared_ptr<Node> c) : StmtNode(kIfStmtNode), hasElse(false) {
		condition = std::move(c);
		condition->parent = this;
		block1 = std::make_shared<BlockNode>();
		block1->parent = this;
		block2 = std::make_shared<BlockNode>();
		block2->parent = this;
	}
	virtual ~IfStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* RepeatWhileStmtNode */

struct RepeatWhileStmtNode : LoopNode {
	std::shared_ptr<Node> condition;
	std::shared_ptr<BlockNode> block;

	RepeatWhileStmtNode(uint32_t startIndex, std::shared_ptr<Node> c)
		: LoopNode(kRepeatWhileStmtNode, startIndex) {
		condition = std::move(c);
		condition->parent = this;
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~RepeatWhileStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* RepeatWithInStmtNode */

struct RepeatWithInStmtNode : LoopNode {
	Common::String varName;
	std::shared_ptr<Node> list;
	std::shared_ptr<BlockNode> block;

	RepeatWithInStmtNode(uint32_t startIndex, Common::String v, std::shared_ptr<Node> l)
		: LoopNode(kRepeatWithInStmtNode, startIndex) {
		varName = v;
		list = std::move(l);
		list->parent = this;
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~RepeatWithInStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* RepeatWithToStmtNode */

struct RepeatWithToStmtNode : LoopNode {
	Common::String varName;
	std::shared_ptr<Node> start;
	bool up;
	std::shared_ptr<Node> end;
	std::shared_ptr<BlockNode> block;

	RepeatWithToStmtNode(uint32_t startIndex, Common::String v, std::shared_ptr<Node> s, bool up, std::shared_ptr<Node> e)
		: LoopNode(kRepeatWithToStmtNode, startIndex), up(up) {
		varName = v;
		start = std::move(s);
		start->parent = this;
		end = std::move(e);
		end->parent = this;
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~RepeatWithToStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* CaseLabelNode */

struct CaseLabelNode : LabelNode {
	std::shared_ptr<Node> value;
	CaseExpect expect;

	std::shared_ptr<CaseLabelNode> nextOr;

	std::shared_ptr<CaseLabelNode> nextLabel;
	std::shared_ptr<BlockNode> block;

	CaseLabelNode(std::shared_ptr<Node> v, CaseExpect e) : LabelNode(kCaseLabelNode), expect(e) {
		value = std::move(v);
		value->parent = this;
	}
	virtual ~CaseLabelNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* OtherwiseNode */

struct OtherwiseNode : LabelNode {
	std::shared_ptr<BlockNode> block;

	OtherwiseNode() : LabelNode(kOtherwiseNode) {
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~OtherwiseNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* EndCaseNode */

struct EndCaseNode : LabelNode {
	EndCaseNode() : LabelNode(kEndCaseNode) {}
	virtual ~EndCaseNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* CaseStmtNode */

struct CaseStmtNode : StmtNode {
	std::shared_ptr<Node> value;
	std::shared_ptr<CaseLabelNode> firstLabel;
	std::shared_ptr<OtherwiseNode> otherwise;

	// for use during translation:
	int32_t endPos = -1;
	int32_t potentialOtherwisePos = -1;

	CaseStmtNode(std::shared_ptr<Node> v) : StmtNode(kCaseStmtNode) {
		value = std::move(v);
		value->parent = this;
	}
	virtual ~CaseStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	void addOtherwise();
};

/* TellStmtNode */

struct TellStmtNode : StmtNode {
	std::shared_ptr<Node> window;
	std::shared_ptr<BlockNode> block;

	TellStmtNode(std::shared_ptr<Node> w) : StmtNode(kTellStmtNode) {
		window = std::move(w);
		window->parent = this;
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~TellStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* SoundCmdStmtNode */

struct SoundCmdStmtNode : StmtNode {
	Common::String cmd;
	std::shared_ptr<Node> argList;

	SoundCmdStmtNode(Common::String c, std::shared_ptr<Node> a) : StmtNode(kSoundCmdStmtNode) {
		cmd = c;
		argList = std::move(a);
		argList->parent = this;
	}
	virtual ~SoundCmdStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* PlayCmdStmtNode */

struct PlayCmdStmtNode : StmtNode {
	std::shared_ptr<Node> argList;

	PlayCmdStmtNode(std::shared_ptr<Node> a) : StmtNode(kPlayCmdStmtNode) {
		argList = std::move(a);
		argList->parent = this;
	}
	virtual ~PlayCmdStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* CallNode */

struct CallNode : Node {
	Common::String name;
	std::shared_ptr<Node> argList;

	CallNode(Common::String n, std::shared_ptr<Node> a) : Node(kCallNode) {
		name = n;
		argList = std::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~CallNode() = default;
	bool noParens() const;
	bool isMemberExpr() const;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjCallNode */

struct ObjCallNode : Node {
	Common::String name;
	std::shared_ptr<Node> argList;

	ObjCallNode(Common::String n, std::shared_ptr<Node> a) : Node(kObjCallNode) {
		name = n;
		argList = std::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~ObjCallNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjCallV4Node */

struct ObjCallV4Node : Node {
	std::shared_ptr<Node> obj;
	std::shared_ptr<Node> argList;

	ObjCallV4Node(std::shared_ptr<Node> o, std::shared_ptr<Node> a) : Node(kObjCallV4Node) {
		obj = o;
		argList = std::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~ObjCallV4Node() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* TheExprNode */

struct TheExprNode : ExprNode {
	Common::String prop;

	TheExprNode(Common::String p) : ExprNode(kTheExprNode), prop(p) {}
	virtual ~TheExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* LastStringChunkExprNode */

struct LastStringChunkExprNode : ExprNode {
	ChunkExprType type;
	std::shared_ptr<Node> obj;

	LastStringChunkExprNode(ChunkExprType t, std::shared_ptr<Node> o)
		: ExprNode(kLastStringChunkExprNode), type(t) {
		obj = std::move(o);
		obj->parent = this;
	}
	virtual ~LastStringChunkExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* StringChunkCountExprNode */

struct StringChunkCountExprNode : ExprNode {
	ChunkExprType type;
	std::shared_ptr<Node> obj;

	StringChunkCountExprNode(ChunkExprType t, std::shared_ptr<Node> o)
		: ExprNode(kStringChunkCountExprNode), type(t) {
		obj = std::move(o);
		obj->parent = this;
	}
	virtual ~StringChunkCountExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* MenuPropExprNode */

struct MenuPropExprNode : ExprNode {
	std::shared_ptr<Node> menuID;
	unsigned int prop;

	MenuPropExprNode(std::shared_ptr<Node> m, unsigned int p)
		: ExprNode(kMenuPropExprNode), prop(p) {
		menuID = std::move(m);
		menuID->parent = this;
	}
	virtual ~MenuPropExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* MenuItemPropExprNode */

struct MenuItemPropExprNode : ExprNode {
	std::shared_ptr<Node> menuID;
	std::shared_ptr<Node> itemID;
	unsigned int prop;

	MenuItemPropExprNode(std::shared_ptr<Node> m, std::shared_ptr<Node> i, unsigned int p)
		: ExprNode(kMenuItemPropExprNode), prop(p) {
		menuID = std::move(m);
		menuID->parent = this;
		itemID = std::move(i);
		itemID->parent = this;
	}
	virtual ~MenuItemPropExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* SoundPropExprNode */

struct SoundPropExprNode : ExprNode {
	std::shared_ptr<Node> soundID;
	unsigned int prop;

	SoundPropExprNode(std::shared_ptr<Node> s, unsigned int p)
		: ExprNode(kSoundPropExprNode), prop(p) {
		soundID = std::move(s);
		soundID->parent = this;
	}
	virtual ~SoundPropExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* SpritePropExprNode */

struct SpritePropExprNode : ExprNode {
	std::shared_ptr<Node> spriteID;
	unsigned int prop;

	SpritePropExprNode(std::shared_ptr<Node> s, unsigned int p)
		: ExprNode(kSpritePropExprNode), prop(p) {
		spriteID = std::move(s);
		spriteID->parent = this;
	}
	virtual ~SpritePropExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* ThePropExprNode */

struct ThePropExprNode : ExprNode {
	std::shared_ptr<Node> obj;
	Common::String prop;

	ThePropExprNode(std::shared_ptr<Node> o, Common::String p)
		: ExprNode(kThePropExprNode), prop(p) {
		obj = std::move(o);
		obj->parent = this;
	}
	virtual ~ThePropExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* ObjPropExprNode */

struct ObjPropExprNode : ExprNode {
	std::shared_ptr<Node> obj;
	Common::String prop;

	ObjPropExprNode(std::shared_ptr<Node> o, Common::String p)
		: ExprNode(kObjPropExprNode), prop(p) {
		obj = std::move(o);
		obj->parent = this;
	}
	virtual ~ObjPropExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjBracketExprNode */

struct ObjBracketExprNode : ExprNode {
	std::shared_ptr<Node> obj;
	std::shared_ptr<Node> prop;

	ObjBracketExprNode(std::shared_ptr<Node> o, std::shared_ptr<Node> p)
		: ExprNode(kObjBracketExprNode) {
		obj = std::move(o);
		obj->parent = this;
		prop = std::move(p);
		prop->parent = this;
	}
	virtual ~ObjBracketExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ObjPropIndexExprNode */

struct ObjPropIndexExprNode : ExprNode {
	std::shared_ptr<Node> obj;
	Common::String prop;
	std::shared_ptr<Node> index;
	std::shared_ptr<Node> index2;

	ObjPropIndexExprNode(std::shared_ptr<Node> o, Common::String p, std::shared_ptr<Node> i, std::shared_ptr<Node> i2)
		: ExprNode(kObjPropIndexExprNode), prop(p) {
		obj = std::move(o);
		obj->parent = this;
		index = std::move(i);
		index->parent = this;
		if (i2) {
			index2 = std::move(i2);
			index2->parent = this;
		}
	}
	virtual ~ObjPropIndexExprNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	virtual bool hasSpaces(bool dot);
};

/* ExitRepeatStmtNode */

struct ExitRepeatStmtNode : StmtNode {
	ExitRepeatStmtNode() : StmtNode(kExitRepeatStmtNode) {}
	virtual ~ExitRepeatStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* NextRepeatStmtNode */

struct NextRepeatStmtNode : StmtNode {
	NextRepeatStmtNode() : StmtNode(kNextRepeatStmtNode) {}
	virtual ~NextRepeatStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* PutStmtNode */

struct PutStmtNode : StmtNode {
	PutType type;
	std::shared_ptr<Node> variable;
	std::shared_ptr<Node> value;

	PutStmtNode(PutType t, std::shared_ptr<Node> var, std::shared_ptr<Node> val)
		: StmtNode(kPutStmtNode), type(t) {
		variable = std::move(var);
		variable->parent = this;
		value = std::move(val);
		value->parent = this;
	}
	virtual ~PutStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* WhenStmtNode */

struct WhenStmtNode : StmtNode {
	int event;
	Common::String script;

	WhenStmtNode(int e, Common::String s)
		: StmtNode(kWhenStmtNode), event(e), script(s) {}
	virtual ~WhenStmtNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* NewObjNode */

struct NewObjNode : ExprNode {
	Common::String objType;
	std::shared_ptr<Node> objArgs;

	NewObjNode(Common::String o, std::shared_ptr<Node> args) : ExprNode(kNewObjNode), objType(o), objArgs(args) {}
	virtual ~NewObjNode() = default;
	virtual void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
};

/* AST */

struct AST {
	std::shared_ptr<HandlerNode> root;
	BlockNode *currentBlock;

	AST(Handler *handler){
		root = std::make_shared<HandlerNode>(handler);
		currentBlock = root->block.get();
	}

	void writeScriptText(Common::CodeWriter &code, bool dot, bool sum) const;
	void addStatement(std::shared_ptr<Node> statement);
	void enterBlock(BlockNode *block);
	void exitBlock();
};

} // namespace LingoDec

#endif // LINGODEC_AST_H
