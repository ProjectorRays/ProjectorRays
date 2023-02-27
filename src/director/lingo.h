/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_LINGO_H
#define DIRECTOR_LINGO_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Common {
class JSONWriter;
class ReadStream;
}

namespace Director {

struct AST;
struct Bytecode;
struct CaseLabelNode;
struct LoopNode;
struct Node;
struct RepeatWithInStmtNode;
struct ScriptChunk;

const char kLingoLineEnding = '\r';

enum OpCode {
	// single-byte
	kOpRet				= 0x01,
	kOpRetFactory		= 0x02,
	kOpPushZero			= 0x03,
	kOpMul				= 0x04,
	kOpAdd				= 0x05,
	kOpSub				= 0x06,
	kOpDiv				= 0x07,
	kOpMod				= 0x08,
	kOpInv				= 0x09,
	kOpJoinStr			= 0x0a,
	kOpJoinPadStr		= 0x0b,
	kOpLt				= 0x0c,
	kOpLtEq				= 0x0d,
	kOpNtEq				= 0x0e,
	kOpEq				= 0x0f,
	kOpGt				= 0x10,
	kOpGtEq				= 0x11,
	kOpAnd				= 0x12,
	kOpOr				= 0x13,
	kOpNot				= 0x14,
	kOpContainsStr		= 0x15,
	kOpContains0Str		= 0x16,
	kOpGetChunk			= 0x17,
	kOpHiliteChunk		= 0x18,
	kOpOntoSpr			= 0x19,
	kOpIntoSpr			= 0x1a,
	kOpGetField			= 0x1b,
	kOpStartTell		= 0x1c,
	kOpEndTell			= 0x1d,
	kOpPushList			= 0x1e,
	kOpPushPropList		= 0x1f,
	kOpSwap				= 0x21,

	// multi-byte
	kOpPushInt8			= 0x41,
	kOpPushArgListNoRet	= 0x42,
	kOpPushArgList		= 0x43,
	kOpPushCons			= 0x44,
	kOpPushSymb			= 0x45,
	kOpPushVarRef		= 0x46,
	kOpGetGlobal2		= 0x48,
	kOpGetGlobal		= 0x49,
	kOpGetProp			= 0x4a,
	kOpGetParam			= 0x4b,
	kOpGetLocal			= 0x4c,
	kOpSetGlobal2		= 0x4e,
	kOpSetGlobal		= 0x4f,
	kOpSetProp			= 0x50,
	kOpSetParam			= 0x51,
	kOpSetLocal			= 0x52,
	kOpJmp				= 0x53,
	kOpEndRepeat		= 0x54,
	kOpJmpIfZ			= 0x55,
	kOpLocalCall		= 0x56,
	kOpExtCall			= 0x57,
	kOpObjCallV4		= 0x58,
	kOpPut				= 0x59,
	kOpPutChunk			= 0x5a,
	kOpDeleteChunk		= 0x5b,
	kOpGet				= 0x5c,
	kOpSet				= 0x5d,
	kOpGetMovieProp		= 0x5f,
	kOpSetMovieProp		= 0x60,
	kOpGetObjProp		= 0x61,
	kOpSetObjProp		= 0x62,
	kOpTellCall			= 0x63,
	kOpPeek				= 0x64,
	kOpPop				= 0x65,
	kOpTheBuiltin		= 0x66,
	kOpObjCall			= 0x67,
	kOpPushChunkVarRef	= 0x6d,
	kOpPushInt16		= 0x6e,
	kOpPushInt32		= 0x6f,
	kOpGetChainedProp	= 0x70,
	kOpPushFloat32		= 0x71,
	kOpGetTopLevelProp	= 0x72,
	kOpNewObj			= 0x73
};

enum DatumType {
	kDatumVoid,
	kDatumSymbol,
	kDatumVarRef,
	kDatumString,
	kDatumInt,
	kDatumFloat,
	kDatumList,
	kDatumArgList,
	kDatumArgListNoRet,
	kDatumPropList
};

enum ChunkExprType {
	kChunkChar	= 0x01,
	kChunkWord	= 0x02,
	kChunkItem	= 0x03,
	kChunkLine	= 0x04
};

enum PutType {
	kPutInto	= 0x01,
	kPutAfter	= 0x02,
	kPutBefore	= 0x03
};

enum NodeType {
	kNoneNode,
	kErrorNode,
	kTempNode,
	kCommentNode,
	kLiteralNode,
	kBlockNode,
	kHandlerNode,
	kExitStmtNode,
	kInverseOpNode,
	kNotOpNode,
	kBinaryOpNode,
	kChunkExprNode,
	kChunkHiliteStmtNode,
	kChunkDeleteStmtNode,
	kSpriteIntersectsExprNode,
	kSpriteWithinExprNode,
	kMemberExprNode,
	kVarNode,
	kAssignmentStmtNode,
	kIfStmtNode,
	kRepeatWhileStmtNode,
	kRepeatWithInStmtNode,
	kRepeatWithToStmtNode,
	kCaseStmtNode,
	kCaseLabelNode,
	kOtherwiseNode,
	kEndCaseNode,
	kTellStmtNode,
	kSoundCmdStmtNode,
	kCallNode,
	kObjCallNode,
	kObjCallV4Node,
	kTheExprNode,
	kLastStringChunkExprNode,
	kStringChunkCountExprNode,
	kMenuPropExprNode,
	kMenuItemPropExprNode,
	kSoundPropExprNode,
	kSpritePropExprNode,
	kThePropExprNode,
	kObjPropExprNode,
	kObjBracketExprNode,
	kObjPropIndexExprNode,
	kExitRepeatStmtNode,
	kNextRepeatStmtNode,
	kPutStmtNode,
	kWhenStmtNode,
	kNewObjNode
};

enum BytecodeTag {
	kTagNone,
	kTagSkip,
	kTagRepeatWhile,
	kTagRepeatWithIn,
	kTagRepeatWithTo,
	kTagRepeatWithDownTo,
	kTagNextRepeatTarget,
	kTagEndCase
};

enum CaseExpect {
	kCaseExpectEnd,
	kCaseExpectOr,
	kCaseExpectNext,
	kCaseExpectOtherwise
};

enum ScriptFlag {
	kScriptFlagUnused		= (1 << 0x0),
	kScriptFlagFuncsGlobal	= (1 << 0x1),
	kScriptFlagVarsGlobal	= (1 << 0x2),	// Occurs in event scripts (which have no local vars). Correlated with use of alternate global var opcodes.
	kScriptFlagUnk3			= (1 << 0x3),
	kScriptFlagFactoryDef	= (1 << 0x4),
	kScriptFlagUnk5			= (1 << 0x5),
	kScriptFlagUnk6			= (1 << 0x6),
	kScriptFlagUnk7			= (1 << 0x7),
	kScriptFlagHasFactory	= (1 << 0x8),
	kScriptFlagEventScript	= (1 << 0x9),
	kScriptFlagEventScript2	= (1 << 0xa),
	kScriptFlagUnkB			= (1 << 0xb),
	kScriptFlagUnkC			= (1 << 0xc),
	kScriptFlagUnkD			= (1 << 0xd),
	kScriptFlagUnkE			= (1 << 0xe),
	kScriptFlagUnkF			= (1 << 0xf)
};

/* Lingo */

struct Lingo {
	static std::map<unsigned int, std::string> opcodeNames;
	static std::map<unsigned int, std::string> binaryOpNames;
	static std::map<unsigned int, std::string> chunkTypeNames;
	static std::map<unsigned int, std::string> putTypeNames;
	static std::map<unsigned int, std::string> moviePropertyNames;
	static std::map<unsigned int, std::string> whenEventNames;
	static std::map<unsigned int, std::string> timeNames;
	static std::map<unsigned int, std::string> menuPropertyNames;
	static std::map<unsigned int, std::string> menuItemPropertyNames;
	static std::map<unsigned int, std::string> soundPropertyNames;
	static std::map<unsigned int, std::string> spritePropertyNames;
	static std::map<unsigned int, std::string> animationPropertyNames;
	static std::map<unsigned int, std::string> animation2PropertyNames;
	static std::map<unsigned int, std::string> memberPropertyNames;

	static std::string getOpcodeName(uint8_t id);
	static std::string getName(const std::map<unsigned int, std::string> &nameMap, unsigned int id);
};

/* Datum */

struct Datum {
	DatumType type;
	int i;
	double f;
	std::string s;
	std::vector<std::shared_ptr<Node>> l;

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
	Datum(DatumType t, std::string val) {
		type = t;
		s = val;
	}
	Datum(DatumType t, std::vector<std::shared_ptr<Node>> val) {
		type = t;
		l = val;
	}

	int toInt();
	std::string toString(bool dot, bool sum);
	void writeJSON(Common::JSONWriter &json) const;
};

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

	ScriptChunk *script;
	std::vector<Bytecode> bytecodeArray;
	std::map<uint32_t, size_t> bytecodePosMap;
	std::vector<std::string> argumentNames;
	std::vector<std::string> localNames;
	std::vector<std::string> globalNames;
	std::string name;

	std::vector<std::shared_ptr<Node>> stack;
	std::unique_ptr<AST> ast;

	bool isGenericEvent = false;

	Handler(ScriptChunk *s) {
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
	void translate();
	uint32_t translateBytecode(Bytecode &bytecode, uint32_t index);
	std::string bytecodeText();
	void writeJSON(Common::JSONWriter &json) const;
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
	virtual bool hasSpaces(bool dot);
};

/* CommentNode */

struct CommentNode : Node {
	std::string text;

	CommentNode(std::string t) : Node(kCommentNode), text(t) {}
	virtual ~CommentNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* LiteralNode */

struct LiteralNode : ExprNode {
	std::shared_ptr<Datum> value;

	LiteralNode(std::shared_ptr<Datum> d) : ExprNode(kLiteralNode) {
		value = std::move(d);
	}
	virtual ~LiteralNode() = default;
	virtual std::string toString(bool dot, bool sum);
	virtual std::shared_ptr<Datum> getValue();
	virtual bool hasSpaces(bool dot);
};

/* BlockNode */

struct BlockNode : Node {
	std::vector<std::shared_ptr<Node>> children;

	// for use during translation:
	uint32_t endPos;
	CaseLabelNode *currentCaseLabel;

	BlockNode() : Node(kBlockNode), endPos(-1), currentCaseLabel(nullptr) {}
	virtual ~BlockNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
};

/* ExitStmtNode */

struct ExitStmtNode : StmtNode {
	ExitStmtNode() : StmtNode(kExitStmtNode) {}
	virtual ~ExitStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* InverseOpNode */

struct InverseOpNode : ExprNode {
	std::shared_ptr<Node> operand;

	InverseOpNode(std::shared_ptr<Node> o) : ExprNode(kInverseOpNode) {
		operand = std::move(o);
		operand->parent = this;
	}
	virtual ~InverseOpNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* NotOpNode */

struct NotOpNode : ExprNode {
	std::shared_ptr<Node> operand;

	NotOpNode(std::shared_ptr<Node> o) : ExprNode(kNotOpNode) {
		operand = std::move(o);
		operand->parent = this;
	}
	virtual ~NotOpNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
	virtual unsigned int getPrecedence();
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
	virtual std::string toString(bool dot, bool sum);
};

/* ChunkHiliteStmtNode */

struct ChunkHiliteStmtNode : StmtNode {
	std::shared_ptr<Node> chunk;

	ChunkHiliteStmtNode(std::shared_ptr<Node> c) : StmtNode(kChunkHiliteStmtNode) {
		chunk = std::move(c);
		chunk->parent = this;
	}
	virtual ~ChunkHiliteStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* ChunkDeleteStmtNode */

struct ChunkDeleteStmtNode : StmtNode {
	std::shared_ptr<Node> chunk;

	ChunkDeleteStmtNode(std::shared_ptr<Node> c) : StmtNode(kChunkDeleteStmtNode) {
		chunk = std::move(c);
		chunk->parent = this;
	}
	virtual ~ChunkDeleteStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
};

/* MemberExprNode */

struct MemberExprNode : ExprNode {
	std::string type;
	std::shared_ptr<Node> memberID;
	std::shared_ptr<Node> castID;

	MemberExprNode(std::string type, std::shared_ptr<Node> memberID, std::shared_ptr<Node> castID)
		: ExprNode(kMemberExprNode), type(type) {
		this->memberID = std::move(memberID);
		this->memberID->parent = this;
		if (castID) {
			this->castID = std::move(castID);
			this->castID->parent = this;
		}
	}
	virtual ~MemberExprNode() = default;
	virtual std::string toString(bool dot, bool sum);
	virtual bool hasSpaces(bool dot);
};

/* VarNode */

struct VarNode : ExprNode {
	std::string varName;

	VarNode(std::string v) : ExprNode(kVarNode), varName(v) {}
	virtual ~VarNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
};

/* RepeatWithInStmtNode */

struct RepeatWithInStmtNode : LoopNode {
	std::string varName;
	std::shared_ptr<Node> list;
	std::shared_ptr<BlockNode> block;

	RepeatWithInStmtNode(uint32_t startIndex, std::string v, std::shared_ptr<Node> l)
		: LoopNode(kRepeatWithInStmtNode, startIndex) {
		varName = v;
		list = std::move(l);
		list->parent = this;
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~RepeatWithInStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* RepeatWithToStmtNode */

struct RepeatWithToStmtNode : LoopNode {
	std::string varName;
	std::shared_ptr<Node> start;
	bool up;
	std::shared_ptr<Node> end;
	std::shared_ptr<BlockNode> block;

	RepeatWithToStmtNode(uint32_t startIndex, std::string v, std::shared_ptr<Node> s, bool up, std::shared_ptr<Node> e)
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
};

/* OtherwiseNode */

struct OtherwiseNode : LabelNode {
	std::shared_ptr<BlockNode> block;

	OtherwiseNode() : LabelNode(kOtherwiseNode) {
		block = std::make_shared<BlockNode>();
		block->parent = this;
	}
	virtual ~OtherwiseNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* EndCaseNode */

struct EndCaseNode : LabelNode {
	EndCaseNode() : LabelNode(kEndCaseNode) {}
	virtual ~EndCaseNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
};

/* SoundCmdStmtNode */
struct SoundCmdStmtNode : StmtNode {
	std::string cmd;
	std::shared_ptr<Node> argList;

	SoundCmdStmtNode(std::string c, std::shared_ptr<Node> a) : StmtNode(kSoundCmdStmtNode) {
		cmd = c;
		argList = std::move(a);
		argList->parent = this;
	}
	virtual ~SoundCmdStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* CallNode */

struct CallNode : Node {
	std::string name;
	std::shared_ptr<Node> argList;

	CallNode(std::string n, std::shared_ptr<Node> a) : Node(kCallNode) {
		name = n;
		argList = std::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~CallNode() = default;
	bool noParens();
	bool isMemberExpr();
	virtual std::string toString(bool dot, bool sum);
	virtual bool hasSpaces(bool dot);
};

/* ObjCallNode */

struct ObjCallNode : Node {
	std::string name;
	std::shared_ptr<Node> argList;

	ObjCallNode(std::string n, std::shared_ptr<Node> a) : Node(kObjCallNode) {
		name = n;
		argList = std::move(a);
		argList->parent = this;
		if (argList->getValue()->type == kDatumArgListNoRet)
			isStatement = true;
		else
			isExpression = true;
	}
	virtual ~ObjCallNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
	virtual bool hasSpaces(bool dot);
};

/* TheExprNode */

struct TheExprNode : ExprNode {
	std::string prop;

	TheExprNode(std::string p) : ExprNode(kTheExprNode), prop(p) {}
	virtual ~TheExprNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
};

/* ThePropExprNode */

struct ThePropExprNode : ExprNode {
	std::shared_ptr<Node> obj;
	std::string prop;

	ThePropExprNode(std::shared_ptr<Node> o, std::string p)
		: ExprNode(kThePropExprNode), prop(p) {
		obj = std::move(o);
		obj->parent = this;
	}
	virtual ~ThePropExprNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* ObjPropExprNode */

struct ObjPropExprNode : ExprNode {
	std::shared_ptr<Node> obj;
	std::string prop;

	ObjPropExprNode(std::shared_ptr<Node> o, std::string p)
		: ExprNode(kObjPropExprNode), prop(p) {
		obj = std::move(o);
		obj->parent = this;
	}
	virtual ~ObjPropExprNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
	virtual bool hasSpaces(bool dot);
};

/* ObjPropIndexExprNode */

struct ObjPropIndexExprNode : ExprNode {
	std::shared_ptr<Node> obj;
	std::string prop;
	std::shared_ptr<Node> index;
	std::shared_ptr<Node> index2;

	ObjPropIndexExprNode(std::shared_ptr<Node> o, std::string p, std::shared_ptr<Node> i, std::shared_ptr<Node> i2)
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
	virtual std::string toString(bool dot, bool sum);
	virtual bool hasSpaces(bool dot);
};

/* ExitRepeatStmtNode */

struct ExitRepeatStmtNode : StmtNode {
	ExitRepeatStmtNode() : StmtNode(kExitRepeatStmtNode) {}
	virtual ~ExitRepeatStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* NextRepeatStmtNode */

struct NextRepeatStmtNode : StmtNode {
	NextRepeatStmtNode() : StmtNode(kNextRepeatStmtNode) {}
	virtual ~NextRepeatStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
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
	virtual std::string toString(bool dot, bool sum);
};

/* WhenStmtNode */

struct WhenStmtNode : StmtNode {
	int event;
	std::string script;

	WhenStmtNode(int e, std::string s)
		: StmtNode(kWhenStmtNode), event(e), script(s) {}
	virtual ~WhenStmtNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* NewObjNode */

struct NewObjNode : ExprNode {
	std::string objType;
	std::shared_ptr<Node> objArgs;

	NewObjNode(std::string o, std::shared_ptr<Node> args) : ExprNode(kNewObjNode), objType(o), objArgs(args) {}
	virtual ~NewObjNode() = default;
	virtual std::string toString(bool dot, bool sum);
};

/* AST */

struct AST {
	std::shared_ptr<HandlerNode> root;
	BlockNode *currentBlock;

	AST(Handler *handler){
		root = std::make_shared<HandlerNode>(handler);
		currentBlock = root->block.get();
	}

	std::string toString(bool dot, bool sum);
	void addStatement(std::shared_ptr<Node> statement);
	void enterBlock(BlockNode *block);
	void exitBlock();
};

} // namespace Director

#endif // DIRECTOR_LINGO_H
