#ifndef LINGO_H
#define LINGO_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ProjectorRays {

enum IfType {
    kIf,
    kIfElse,
    kRepeatWhile
};

struct AST;
struct BlockNode;
struct Bytecode;
struct Node;
struct ReadStream;
struct ScriptChunk;

typedef unsigned int uint;

enum OpCode {
    // single-byte
    kOpRet              = 0x01,
    kOpPushZero         = 0x03,
    kOpMul              = 0x04,
    kOpAdd              = 0x05,
    kOpSub              = 0x06,
    kOpDiv              = 0x07,
    kOpMod              = 0x08,
    kOpInv              = 0x09,
    kOpJoinStr          = 0x0a,
    kOpJoinPadStr       = 0x0b,
    kOpLt               = 0x0c,
    kOpLtEq             = 0x0d,
    kOpNtEq             = 0x0e,
    kOpEq               = 0x0f,
    kOpGt               = 0x10,
    kOpGtEq             = 0x11,
    kOpAnd              = 0x12,
    kOpOr               = 0x13,
    kOpNot              = 0x14,
    kOpContainsStr      = 0x15,
    kOpContains0Str     = 0x16,
    kOpSplitStr         = 0x17,
    kOpHiliiteStr       = 0x18,
    kOpOntoSpr          = 0x19,
    kOpIntoSpr          = 0x1a,
    kOpCastStr          = 0x1b,
    kOpStartObj         = 0x1c,
    kOpStopObj          = 0x1d,
    kOpPushList         = 0x1e,
    kOpPushPropList     = 0x1f,

    // multi-byte
    kOpPushInt01        = 0x41,
    kOpPushArgListNoRet = 0x42,
    kOpPushArgList      = 0x43,
    kOpPushCons         = 0x44,
    kOpPushSymb         = 0x45,
    kOpGetGlobal        = 0x49,
    kOpGetProp          = 0x4a,
    kOpGetParam         = 0x4b,
    kOpGetLocal         = 0x4c,
    kOpSetGlobal        = 0x4f,
    kOpSetProp          = 0x50,
    kOpSetParam         = 0x51,
    kOpSetLocal         = 0x52,
    kOpJmp              = 0x53,
    kOpEndRepeat        = 0x54,
    kOpJmpIfZ           = 0x55,
    kOpCallLocal        = 0x56,
    kOpCallExt          = 0x57,
    kOpCallObjOld       = 0x58,
    kOp59XX             = 0x59,
    kOp5BXX             = 0x5b,
    kOpGet              = 0x5c,
    kOpSet              = 0x5d,
    kOpGetMovieProp     = 0x5f,
    kOpSetMovieProp     = 0x60,
    kOpGetObjProp       = 0x61,
    kOpSetObjProp       = 0x62,
    kOpGetMovieInfo     = 0x66,
    kOpCallObj          = 0x67,
    kOpPushInt2E        = 0x6e
};

enum DatumType {
    kDatumVoid,
    kDatumSymbol,
    kDatumString,
    kDatumInt,
    kDatumFloat,
    kDatumList,
    kDatumArgList,
    kDatumArgListNoRet,
    kDatumPropList
};

enum ChunkType {
    kChunkChar  = 0x01,
    kChunkWord  = 0x02,
    kChunkItem  = 0x03,
    kChunkLine  = 0x04
};

enum NodeType {
    kNoneNode,
    kErrorNode,
    kCommentNode,
    kLiteralNode,
    kBlockNode,
    kHandlerNode,
    kExitStmtNode,
    kInverseOpNode,
    kNotOpNode,
    kBinaryOpNode,
    kStringSplitExprNode,
    kStringHiliteStmtNode,
    kSpriteIntersectsExprNode,
    kSpriteWithinExprNode,
    kFieldExprNode,
    kVarNode,
    kAssignmentStmtNode,
    kIfStmtNode,
    kCallNode,
    kObjCallNode,
    kTheExprNode,
    kLastStringChunkExprNode,
    kStringChunkCountExprNode,
    kMenuPropExprNode,
    kMenuItemPropExprNode,
    kSoundPropExprNode,
    kSpritePropExprNode,
    kCastPropExprNode,
    kFieldPropExprNode,
    kObjPropExprNode,
    kExitRepeatStmtNode,
    kNextRepeatStmtNode
};

/* Lingo */

struct Lingo {
    static std::map<uint, std::string> opcodeNames;
    static std::map<uint, std::string> binaryOpNames;
    static std::map<uint, std::string> moviePropertyNames00;
    static std::map<uint, std::string> timeNames;
    static std::map<uint, std::string> chunkTypeNames;
    static std::map<uint, std::string> menuPropertyNames;
    static std::map<uint, std::string> menuItemPropertyNames;
    static std::map<uint, std::string> soundPropertyNames;
    static std::map<uint, std::string> spritePropertyNames;
    static std::map<uint, std::string> moviePropertyNames07;
    static std::map<uint, std::string> moviePropertyNames08;
    static std::map<uint, std::string> castPropertyNames09;
    static std::map<uint, std::string> fieldPropertyNames;
    static std::map<uint, std::string> castPropertyNames0D;

    static std::string getOpcodeName(uint8_t id);
    static std::string getName(const std::map<uint, std::string> &nameMap, uint id);
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
    std::string toString(bool summary);
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
    uint16_t unknown0Count;
    uint32_t unknown0Offset;
    uint32_t unknown1;
    uint16_t unknown2;
    uint16_t lineCount;
    uint32_t lineOffset;
    uint32_t stackHeight;

    std::vector<int16_t> argumentNameIDs;
    std::vector<int16_t> localNameIDs;

    ScriptChunk *script;
    std::vector<Bytecode> bytecodeArray;
    std::map<uint32_t, size_t> bytecodePosMap;
    std::vector<std::string> argumentNames;
    std::vector<std::string> localNames;
    std::vector<std::string> globalNames;
    std::string name;

    std::vector<std::shared_ptr<Node>> stack;
    std::unique_ptr<AST> ast;

    Handler(ScriptChunk *s) {
        script = s;
    }

    void readRecord(ReadStream &stream);
    void readData(ReadStream &stream);
    std::vector<int16_t> readVarnamesTable(ReadStream &stream, uint16_t count, uint32_t offset);
    void readNames(const std::vector<std::string> &names);
    std::shared_ptr<Node> pop();
    int variableMultiplier();
    void registerGlobal(std::string name);
    void translate(const std::vector<std::string> &names);
    void translateBytecode(Bytecode &bytecode, size_t pos, const std::vector<std::string> &names);
    std::string bytecodeText();
};

/* Bytecode */

struct Bytecode {
    uint8_t opID;
    OpCode opcode;
    uint32_t obj;
    int32_t pos;
    std::shared_ptr<Node> translation;

    Bytecode(uint8_t op, uint32_t o, int32_t p)
        : opID(op), obj(o), pos(p) {
        opcode = static_cast<OpCode>(op >= 0x40 ? 0x40 + op % 0x40 : op);
    }
};

/* Node */

struct Node {
    NodeType type;
    bool isStatement;
    Node *parent;

    Node(NodeType t, bool s) : type(t), isStatement(s), parent(nullptr) {}
    virtual ~Node() = default;
    virtual std::string toString(bool summary);
    virtual std::shared_ptr<Datum> getValue();
};

/* ErrorNode */

struct ErrorNode : Node {
    ErrorNode() : Node(kErrorNode, false) {};
    virtual ~ErrorNode() = default;
    virtual std::string toString(bool summary);
};

/* CommentNode */

struct CommentNode : Node {
    std::string text;

    CommentNode(std::string t) : Node(kCommentNode, false), text(t) {}
    virtual ~CommentNode() = default;
    virtual std::string toString(bool summary);
};

/* LiteralNode */

struct LiteralNode : Node {
    std::shared_ptr<Datum> value;

    LiteralNode(std::shared_ptr<Datum> d) : Node(kLiteralNode, false) {
        value = std::move(d);
    }
    virtual ~LiteralNode() = default;
    virtual std::string toString(bool summary);
    virtual std::shared_ptr<Datum> getValue();
};

/* BlockNode */

struct BlockNode : Node {
    std::vector<std::shared_ptr<Node>> children;
    int32_t endPos;

    BlockNode() : Node(kBlockNode, false), endPos(-1) {}
    virtual ~BlockNode() = default;
    virtual std::string toString(bool summary);
    void addChild(std::shared_ptr<Node> child);
};

/* HandlerNode */

struct HandlerNode : Node {
    Handler *handler;
    std::shared_ptr<BlockNode> block;

    HandlerNode(Handler *h)
        : Node(kHandlerNode, false), handler(h) {
        block = std::make_shared<BlockNode>();
        block->parent = this;
    }
    virtual ~HandlerNode() = default;
    virtual std::string toString(bool summary);
};

/* ExitStmtNode */

struct ExitStmtNode : Node {
    ExitStmtNode() : Node(kExitStmtNode, true) {}
    virtual ~ExitStmtNode() = default;
    virtual std::string toString(bool summary);
};

/* InverseOpNode */

struct InverseOpNode : Node {
    std::shared_ptr<Node> operand;

    InverseOpNode(std::shared_ptr<Node> o) : Node(kInverseOpNode, false) {
        operand = std::move(o);
        operand->parent = this;
    }
    virtual ~InverseOpNode() = default;
    virtual std::string toString(bool summary);
};

/* NotOpNode */

struct NotOpNode : Node {
    std::shared_ptr<Node> operand;

    NotOpNode(std::shared_ptr<Node> o) : Node(kNotOpNode, false) {
        operand = std::move(o);
        operand->parent = this;
    }
    virtual ~NotOpNode() = default;
    virtual std::string toString(bool summary);
};

/* BinaryOpNode */

struct BinaryOpNode : Node {
    OpCode opcode;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    BinaryOpNode(OpCode op, std::shared_ptr<Node> a, std::shared_ptr<Node> b)
        : Node(kBinaryOpNode, false), opcode(op) {
        left = std::move(a);
        left->parent = this;
        right = std::move(b);
        right->parent = this;
    }
    virtual ~BinaryOpNode() = default;
    virtual std::string toString(bool summary);
};

/* StringSplitExprNode */

struct StringSplitExprNode : Node {
    ChunkType type;
    std::shared_ptr<Node> first;
    std::shared_ptr<Node> last;
    std::shared_ptr<Node> string;

    StringSplitExprNode(ChunkType t, std::shared_ptr<Node> a, std::shared_ptr<Node> b, std::shared_ptr<Node> s)
        : Node(kStringSplitExprNode, false), type(t) {
        first = std::move(a);
        first->parent = this;
        last = std::move(b);
        last->parent = this;
        string = std::move(s);
        string->parent = this;
    }
    virtual ~StringSplitExprNode() = default;
    virtual std::string toString(bool summary);
};

/* StringHiliteStmtNode */

struct StringHiliteStmtNode : Node {
    ChunkType type;
    std::shared_ptr<Node> first;
    std::shared_ptr<Node> last;
    std::shared_ptr<Node> string;

    StringHiliteStmtNode(ChunkType t, std::shared_ptr<Node> a, std::shared_ptr<Node> b, std::shared_ptr<Node> s)
        : Node(kStringHiliteStmtNode, true), type(t) {
        first = std::move(a);
        first->parent = this;
        last = std::move(b);
        last->parent = this;
        string = std::move(s);
        string->parent = this;
    }
    virtual ~StringHiliteStmtNode() = default;
    virtual std::string toString(bool summary);
};

/* SpriteIntersectsExprNode */

struct SpriteIntersectsExprNode : Node {
    std::shared_ptr<Node> firstSprite;
    std::shared_ptr<Node> secondSprite;

    SpriteIntersectsExprNode(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
        : Node(kSpriteIntersectsExprNode, false) {
        firstSprite = std::move(a);
        firstSprite->parent = this;
        secondSprite = std::move(b);
        secondSprite->parent = this;
    }
    virtual ~SpriteIntersectsExprNode() = default;
    virtual std::string toString(bool summary);
};

/* SpriteWithinExprNode */

struct SpriteWithinExprNode : Node {
    std::shared_ptr<Node> firstSprite;
    std::shared_ptr<Node> secondSprite;

    SpriteWithinExprNode(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
        : Node(kSpriteWithinExprNode, false) {
        firstSprite = std::move(a);
        firstSprite->parent = this;
        secondSprite = std::move(b);
        secondSprite->parent = this;
    }
    virtual ~SpriteWithinExprNode() = default;
    virtual std::string toString(bool summary);
};

/* FieldExprNode */

struct FieldExprNode : Node {
    std::shared_ptr<Node> fieldID;

    FieldExprNode(std::shared_ptr<Node> f) : Node(kFieldExprNode, false) {
        fieldID = std::move(f);
        fieldID->parent = this;
    }
    virtual ~FieldExprNode() = default;
    virtual std::string toString(bool summary);
};

/* VarNode */

struct VarNode: Node {
    std::string varName;

    VarNode(std::string v) : Node(kVarNode, false), varName(v) {}
    virtual ~VarNode() = default;
    virtual std::string toString(bool summary);
};

/* AssignmentStmtNode */

struct AssignmentStmtNode : Node {
    std::shared_ptr<Node> variable;
    std::shared_ptr<Node> value;

    AssignmentStmtNode(std::shared_ptr<Node> var, std::shared_ptr<Node> val)
        : Node(kAssignmentStmtNode, true) {
        variable = std::move(var);
        variable->parent = this;
        value = std::move(val);
        value->parent = this;
    }

    virtual ~AssignmentStmtNode() = default;
    virtual std::string toString(bool summary);
};

/* IfStmtNode */

struct IfStmtNode : Node {
    IfType ifType;
    std::shared_ptr<Node> condition;
    std::shared_ptr<BlockNode> block1;
    std::shared_ptr<BlockNode> block2;

    IfStmtNode(std::shared_ptr<Node> c) : Node(kIfStmtNode, true), ifType(kIf) {
        condition = std::move(c);
        condition->parent = this;
        block1 = std::make_shared<BlockNode>();
        block1->parent = this;
        block2 = std::make_shared<BlockNode>();
        block2->parent = this;
    }
    virtual ~IfStmtNode() = default;
    virtual std::string toString(bool summary);
};

/* CallNode */

struct CallNode : Node {
    std::string name;
    std::shared_ptr<Node> argList;

    CallNode(std::string n, std::shared_ptr<Node> a) : Node(kCallNode, false) {
        name = n;
        argList = std::move(a);
        argList->parent = this;
        if (argList->getValue()->type == kDatumArgListNoRet)
            isStatement = true;
    }
    virtual ~CallNode() = default;
    virtual std::string toString(bool summary);
};

/* ObjCallNode */

struct ObjCallNode : Node {
    std::shared_ptr<Node> obj;
    std::string name;
    std::shared_ptr<Node> argList;

    ObjCallNode(std::shared_ptr<Node> o, std::string n, std::shared_ptr<Node> a)
        : Node(kObjCallNode, false) {
        obj = std::move(o);
        obj->parent = this;
        name = n;
        argList = std::move(a);
        argList->parent = this;
        if (argList->getValue()->type == kDatumArgListNoRet)
            isStatement = true;
    }
    virtual ~ObjCallNode() = default;
    virtual std::string toString(bool summary);
};

/* TheExprNode */

struct TheExprNode : Node {
    std::string prop;

    TheExprNode(std::string p) : Node(kTheExprNode, false), prop(p) {}
    virtual ~TheExprNode() = default;
    virtual std::string toString(bool summary);
};

/* LastStringChunkExprNode */

struct LastStringChunkExprNode : Node {
    ChunkType type;
    std::shared_ptr<Node> string;

    LastStringChunkExprNode(ChunkType t, std::shared_ptr<Node> s)
        : Node(kLastStringChunkExprNode, false), type(t) {
        string = std::move(s);
        string->parent = this;
    }
    virtual ~LastStringChunkExprNode() = default;
    virtual std::string toString(bool summary);
};

/* StringChunkCountExprNode */

struct StringChunkCountExprNode : Node {
    ChunkType type;
    std::shared_ptr<Node> string;

    StringChunkCountExprNode(ChunkType t, std::shared_ptr<Node> s)
        : Node(kStringChunkCountExprNode, false), type(t) {
        string = std::move(s);
        string->parent = this;
    }
    virtual ~StringChunkCountExprNode() = default;
    virtual std::string toString(bool summary);
};

/* MenuPropExprNode */

struct MenuPropExprNode : Node {
    std::shared_ptr<Node> menuID;
    uint prop;

    MenuPropExprNode(std::shared_ptr<Node> m, uint p)
        : Node(kMenuPropExprNode, false), prop(p) {
        menuID = std::move(m);
        menuID->parent = this;
    }
    virtual ~MenuPropExprNode() = default;
    virtual std::string toString(bool summary);
};

/* MenuItemPropExprNode */

struct MenuItemPropExprNode : Node {
    std::shared_ptr<Node> menuID;
    std::shared_ptr<Node> itemID;
    uint prop;

    MenuItemPropExprNode(std::shared_ptr<Node> m, std::shared_ptr<Node> i, uint p)
        : Node(kMenuItemPropExprNode, false), prop(p) {
        menuID = std::move(m);
        menuID->parent = this;
        itemID = std::move(i);
        itemID->parent = this;
    }
    virtual ~MenuItemPropExprNode() = default;
    virtual std::string toString(bool summary);
};

/* SoundPropExprNode */

struct SoundPropExprNode : Node {
    std::shared_ptr<Node> soundID;
    uint prop;

    SoundPropExprNode(std::shared_ptr<Node> s, uint p)
        : Node(kSoundPropExprNode, false), prop(p) {
        soundID = std::move(s);
        soundID->parent = this;
    }
    virtual ~SoundPropExprNode() = default;
    virtual std::string toString(bool summary);
};

/* SpritePropExprNode */

struct SpritePropExprNode : Node {
    std::shared_ptr<Node> spriteID;
    uint prop;

    SpritePropExprNode(std::shared_ptr<Node> s, uint p)
        : Node(kSpritePropExprNode, false), prop(p) {
        spriteID = std::move(s);
        spriteID->parent = this;
    }
    virtual ~SpritePropExprNode() = default;
    virtual std::string toString(bool summary);
};

/* CastPropExprNode */

struct CastPropExprNode : Node {
    std::shared_ptr<Node> castID;
    std::string prop;

    CastPropExprNode(std::shared_ptr<Node> c, std::string p)
        : Node(kCastPropExprNode, false), prop(p) {
        castID = std::move(c);
        castID->parent = this;
    }
    virtual ~CastPropExprNode() = default;
    virtual std::string toString(bool summary);
};

/* FieldPropExprNode */

struct FieldPropExprNode : Node {
    std::shared_ptr<Node> fieldID;
    uint prop;

    FieldPropExprNode(std::shared_ptr<Node> f, uint p)
        : Node(kFieldPropExprNode, false), prop(p) {
        fieldID = std::move(f);
        fieldID->parent = this;
    }
    virtual ~FieldPropExprNode() = default;
    virtual std::string toString(bool summary);
};

/* ObjPropExprNode */

struct ObjPropExprNode : Node {
    std::shared_ptr<Node> obj;
    std::string prop;

    ObjPropExprNode(std::shared_ptr<Node> o, std::string p)
        : Node(kObjPropExprNode, false), prop(p) {
        obj = std::move(o);
        obj->parent = this;
    }
    virtual ~ObjPropExprNode() = default;
    virtual std::string toString(bool summary);
};

/* ExitRepeatStmtNode */

struct ExitRepeatStmtNode : Node {
    ExitRepeatStmtNode() : Node(kExitRepeatStmtNode, true) {}
    virtual ~ExitRepeatStmtNode() = default;
    virtual std::string toString(bool summary);
};

/* NextRepeatStmtNode */

struct NextRepeatStmtNode : Node {
    NextRepeatStmtNode() : Node(kNextRepeatStmtNode, true) {}
    virtual ~NextRepeatStmtNode() = default;
    virtual std::string toString(bool summary);
};

/* AST */

struct AST {
    std::shared_ptr<HandlerNode> root;
    BlockNode *currentBlock;

    AST(Handler *handler) {
        root = std::make_shared<HandlerNode>(handler);
        currentBlock = root->block.get();
    }

    std::string toString(bool summary);
    void addStatement(std::shared_ptr<Node> statement);
    void enterBlock(BlockNode *block);
    void exitBlock();
};

}

#endif
