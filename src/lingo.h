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
    std::string toString();
};

/* Handler */

struct Handler {
    uint16_t nameID;
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
    
    std::vector<uint16_t> argumentNameIDs;
    std::vector<uint16_t> localNameIDs;

    std::weak_ptr<ScriptChunk> script;
    std::vector<Bytecode> bytecodeArray;
    std::map<uint32_t, size_t> bytecodePosMap;
    std::vector<std::string> argumentNames;
    std::vector<std::string> localNames;
    std::string name;

    std::vector<std::shared_ptr<Node>> stack;
    std::unique_ptr<AST> ast;

    Handler(std::weak_ptr<ScriptChunk> s) {
        script = std::move(s);
    }

    void readRecord(ReadStream &stream);
    void readData(ReadStream &stream);
    std::vector<uint16_t> readVarnamesTable(ReadStream &stream, uint16_t count, uint32_t offset);
    void readNames(const std::vector<std::string> &names);
    std::shared_ptr<Node> pop();
    void translate(const std::vector<std::string> &names);
    void translateBytecode(Bytecode &bytecode, size_t pos, const std::shared_ptr<ScriptChunk> &scr, const std::vector<std::string> &names);
};

/* Bytecode */

struct Bytecode {
    OpCode opcode;
    uint32_t obj;
    size_t pos;
    std::shared_ptr<Node> translation;

    Bytecode(OpCode op, uint32_t o, size_t p)
        : opcode(op), obj(o), pos(p) {}
};

/* Node */

struct Node {
    NodeType type;
    std::weak_ptr<Node> parent;

    Node(NodeType t) : type(t) {}
    virtual ~Node() = default;
    virtual void connect();
    virtual std::string toString();
    virtual std::shared_ptr<Datum> getValue();
};

/* ErrorNode */

struct ErrorNode : Node {
    ErrorNode() : Node(kErrorNode) {};
    virtual ~ErrorNode() = default;
    virtual std::string toString();
};

/* CommentNode */

struct CommentNode : Node {
    std::string text;

    CommentNode(std::string t) : Node(kCommentNode), text(t) {}
    virtual ~CommentNode() = default;
    virtual std::string toString();
};

/* LiteralNode */

struct LiteralNode : Node {
    std::shared_ptr<Datum> value;

    LiteralNode(std::shared_ptr<Datum> d) : Node(kLiteralNode) {
        value = std::move(d);
    }
    virtual ~LiteralNode() = default;
    virtual std::string toString();
    virtual std::shared_ptr<Datum> getValue();
};

/* BlockNode */

struct BlockNode : Node, std::enable_shared_from_this<BlockNode> {
    std::vector<std::shared_ptr<Node>> children;
    int endPos;

    BlockNode() : Node(kBlockNode), endPos(-1) {}
    virtual ~BlockNode() = default;
    virtual std::string toString();
    void addChild(std::shared_ptr<Node> child);
};

/* HandlerNode */

struct HandlerNode : Node, std::enable_shared_from_this<HandlerNode> {
    std::string name;
    std::vector<std::string> args;
    std::shared_ptr<BlockNode> block;

    HandlerNode(std::string n, std::vector<std::string> a)
        : Node(kHandlerNode), name(n), args(a) {
        block = std::make_shared<BlockNode>();
    }
    virtual ~HandlerNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* ExitStmtNode */

struct ExitStmtNode : Node {
    ExitStmtNode() : Node(kExitStmtNode) {}
    virtual ~ExitStmtNode() = default;
    virtual std::string toString();
};

/* InverseOpNode */

struct InverseOpNode : Node, std::enable_shared_from_this<InverseOpNode> {
    std::shared_ptr<Node> operand;

    InverseOpNode(std::shared_ptr<Node> o) : Node(kInverseOpNode) {
        operand = std::move(o);
    }
    virtual ~InverseOpNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* NotOpNode */

struct NotOpNode : Node, std::enable_shared_from_this<NotOpNode> {
    std::shared_ptr<Node> operand;

    NotOpNode(std::shared_ptr<Node> o) : Node(kNotOpNode) {
        operand = std::move(o);
    }
    virtual ~NotOpNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* BinaryOpNode */

struct BinaryOpNode : Node, std::enable_shared_from_this<BinaryOpNode> {
    OpCode opcode;
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;

    BinaryOpNode(OpCode op, std::shared_ptr<Node> a, std::shared_ptr<Node> b)
        : Node(kBinaryOpNode), opcode(op) {
        left = std::move(a);
        right = std::move(b);
    }
    virtual ~BinaryOpNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* StringSplitExprNode */

struct StringSplitExprNode : Node, std::enable_shared_from_this<StringSplitExprNode> {
    ChunkType type;
    std::shared_ptr<Node> first;
    std::shared_ptr<Node> last;
    std::shared_ptr<Node> string;

    StringSplitExprNode(ChunkType t, std::shared_ptr<Node> a, std::shared_ptr<Node> b, std::shared_ptr<Node> s)
        : Node(kStringSplitExprNode), type(t) {
        first = std::move(a);
        last = std::move(b);
        string = std::move(s);
    }
    virtual ~StringSplitExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* StringHiliteStmtNode */

struct StringHiliteStmtNode : Node, std::enable_shared_from_this<StringHiliteStmtNode> {
    ChunkType type;
    std::shared_ptr<Node> first;
    std::shared_ptr<Node> last;
    std::shared_ptr<Node> string;

    StringHiliteStmtNode(ChunkType t, std::shared_ptr<Node> a, std::shared_ptr<Node> b, std::shared_ptr<Node> s)
        : Node(kStringHiliteStmtNode), type(t) {
        first = std::move(a);
        last = std::move(b);
        string = std::move(s);
    }
    virtual ~StringHiliteStmtNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* SpriteIntersectsExprNode */

struct SpriteIntersectsExprNode : Node, std::enable_shared_from_this<SpriteIntersectsExprNode> {
    std::shared_ptr<Node> firstSprite;
    std::shared_ptr<Node> secondSprite;

    SpriteIntersectsExprNode(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
        : Node(kSpriteIntersectsExprNode) {
        firstSprite = std::move(a);
        secondSprite = std::move(b);
    }
    virtual ~SpriteIntersectsExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* SpriteWithinExprNode */

struct SpriteWithinExprNode : Node, std::enable_shared_from_this<SpriteWithinExprNode> {
    std::shared_ptr<Node> firstSprite;
    std::shared_ptr<Node> secondSprite;

    SpriteWithinExprNode(std::shared_ptr<Node> a, std::shared_ptr<Node> b)
        : Node(kSpriteWithinExprNode) {
        firstSprite = std::move(a);
        secondSprite = std::move(b);
    }
    virtual ~SpriteWithinExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* FieldExprNode */

struct FieldExprNode : Node, std::enable_shared_from_this<FieldExprNode> {
    std::shared_ptr<Node> fieldID;

    FieldExprNode(std::shared_ptr<Node> f) : Node(kFieldExprNode) {
        fieldID = std::move(f);
    }
    virtual ~FieldExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* VarNode */

struct VarNode: Node {
    std::string varName;

    VarNode(std::string v) : Node(kVarNode), varName(v) {}
    virtual ~VarNode() = default;
    virtual std::string toString();
};

/* AssignmentStmtNode */

struct AssignmentStmtNode : Node, std::enable_shared_from_this<AssignmentStmtNode> {
    std::shared_ptr<Node> variable;
    std::shared_ptr<Node> value;

    AssignmentStmtNode(std::shared_ptr<Node> var, std::shared_ptr<Node> val)
        : Node(kAssignmentStmtNode) {
        variable = std::move(var);
        value = std::move(val);
    }

    virtual ~AssignmentStmtNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* IfStmtNode */

struct IfStmtNode : Node, std::enable_shared_from_this<IfStmtNode> {
    IfType ifType;
    std::shared_ptr<Node> condition;
    std::shared_ptr<BlockNode> block1;
    std::shared_ptr<BlockNode> block2;

    IfStmtNode(std::shared_ptr<Node> c) : Node(kIfStmtNode), ifType(kIf) {
        condition = std::move(c);
        block1 = std::make_shared<BlockNode>();
        block2 = std::make_shared<BlockNode>();
    }
    virtual ~IfStmtNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* CallNode */

struct CallNode : Node, std::enable_shared_from_this<CallNode> {
    std::string name;
    std::shared_ptr<Node> argList;

    CallNode(std::string n, std::shared_ptr<Node> a) : Node(kCallNode) {
        name = n;
        argList = std::move(a);
    }
    virtual ~CallNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* ObjCallNode */

struct ObjCallNode : Node, std::enable_shared_from_this<ObjCallNode> {
    std::shared_ptr<Node> obj;
    std::string name;
    std::shared_ptr<Node> argList;

    ObjCallNode(std::shared_ptr<Node> o, std::string n, std::shared_ptr<Node> a)
        : Node(kObjCallNode) {
        obj = std::move(o);
        name = n;
        argList = std::move(a);
    }
    virtual ~ObjCallNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* TheExprNode */

struct TheExprNode : Node {
    std::string prop;

    TheExprNode(std::string p) : Node(kTheExprNode), prop(p) {}
    virtual ~TheExprNode() = default;
    virtual std::string toString();
};

/* LastStringChunkExprNode */

struct LastStringChunkExprNode : Node, std::enable_shared_from_this<LastStringChunkExprNode> {
    ChunkType type;
    std::shared_ptr<Node> string;

    LastStringChunkExprNode(ChunkType t, std::shared_ptr<Node> s)
        : Node(kLastStringChunkExprNode), type(t) {
        string = std::move(s);
    }
    virtual ~LastStringChunkExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* StringChunkCountExprNode */

struct StringChunkCountExprNode : Node, std::enable_shared_from_this<StringChunkCountExprNode> {
    ChunkType type;
    std::shared_ptr<Node> string;

    StringChunkCountExprNode(ChunkType t, std::shared_ptr<Node> s)
        : Node(kStringChunkCountExprNode), type(t) {
        string = std::move(s);
    }
    virtual ~StringChunkCountExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* MenuPropExprNode */

struct MenuPropExprNode : Node, std::enable_shared_from_this<MenuPropExprNode> {
    std::shared_ptr<Node> menuID;
    uint prop;

    MenuPropExprNode(std::shared_ptr<Node> m, uint p)
        : Node(kMenuPropExprNode), prop(p) {
        menuID = std::move(m);
    }
    virtual ~MenuPropExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* MenuItemPropExprNode */

struct MenuItemPropExprNode : Node, std::enable_shared_from_this<MenuItemPropExprNode> {
    std::shared_ptr<Node> menuID;
    std::shared_ptr<Node> itemID;
    uint prop;

    MenuItemPropExprNode(std::shared_ptr<Node> m, std::shared_ptr<Node> i, uint p)
        : Node(kMenuItemPropExprNode), prop(p) {
        menuID = std::move(m);
        itemID = std::move(i);
    }
    virtual ~MenuItemPropExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* SoundPropExprNode */

struct SoundPropExprNode : Node, std::enable_shared_from_this<SoundPropExprNode> {
    std::shared_ptr<Node> soundID;
    uint prop;

    SoundPropExprNode(std::shared_ptr<Node> s, uint p)
        : Node(kSoundPropExprNode), prop(p) {
        soundID = std::move(s);
    }
    virtual ~SoundPropExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* SpritePropExprNode */

struct SpritePropExprNode : Node, std::enable_shared_from_this<SpritePropExprNode> {
    std::shared_ptr<Node> spriteID;
    uint prop;

    SpritePropExprNode(std::shared_ptr<Node> s, uint p)
        : Node(kSpritePropExprNode), prop(p) {
        spriteID = std::move(s);
    }
    virtual ~SpritePropExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* CastPropExprNode */

struct CastPropExprNode : Node, std::enable_shared_from_this<CastPropExprNode> {
    std::shared_ptr<Node> castID;
    std::string prop;

    CastPropExprNode(std::shared_ptr<Node> c, std::string p)
        : Node(kCastPropExprNode), prop(p) {
        castID = std::move(c);
    }
    virtual ~CastPropExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* FieldPropExprNode */

struct FieldPropExprNode : Node, std::enable_shared_from_this<FieldPropExprNode> {
    std::shared_ptr<Node> fieldID;
    uint prop;

    FieldPropExprNode(std::shared_ptr<Node> f, uint p)
        : Node(kFieldPropExprNode), prop(p) {
        fieldID = std::move(f);
    }
    virtual ~FieldPropExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* ObjPropExprNode */

struct ObjPropExprNode : Node, std::enable_shared_from_this<ObjPropExprNode> {
    std::shared_ptr<Node> obj;
    std::string prop;

    ObjPropExprNode(std::shared_ptr<Node> o, std::string p)
        : Node(kObjPropExprNode), prop(p) {
        obj = std::move(o);
    }
    virtual ~ObjPropExprNode() = default;
    virtual void connect();
    virtual std::string toString();
};

/* ExitRepeatStmtNode */

struct ExitRepeatStmtNode : Node {
    ExitRepeatStmtNode() : Node(kExitRepeatStmtNode) {}
    virtual ~ExitRepeatStmtNode() = default;
    virtual std::string toString();
};

/* NextRepeatStmtNode */

struct NextRepeatStmtNode : Node {
    NextRepeatStmtNode() : Node(kNextRepeatStmtNode) {}
    virtual ~NextRepeatStmtNode() = default;
    virtual std::string toString();
};

/* AST */

struct AST {
    std::shared_ptr<HandlerNode> root;
    std::shared_ptr<BlockNode> currentBlock;

    AST(std::string name, std::vector<std::string> args) {
        root = std::make_shared<HandlerNode>(name, args);
        currentBlock = root->block;
    }

    std::string toString();
    void addStatement(std::shared_ptr<Node> statement);
    void enterBlock(std::shared_ptr<BlockNode> block);
    void exitBlock();
};

}

#endif
