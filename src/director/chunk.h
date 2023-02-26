/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef DIRECTOR_CHUNK_H
#define DIRECTOR_CHUNK_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "common/stream.h"
#include "director/castmember.h"
#include "director/subchunk.h"

namespace Common {
class JSONWriter;
}

namespace Director {

struct CastMember;
struct Handler;
struct LiteralStore;
class DirectorFile;

struct CastInfoChunk;
struct CastMemberChunk;
struct ScriptChunk;
struct ScriptContextChunk;
struct ScriptNamesChunk;

enum ChunkType {
	kCastChunk,
	kCastListChunk,
	kCastMemberChunk,
	kCastInfoChunk,
	kConfigChunk,
	kInitialMapChunk,
	kKeyTableChunk,
	kMemoryMapChunk,
	kScriptChunk,
	kScriptContextChunk,
	kScriptNamesChunk
};

struct Chunk {
	DirectorFile *dir;
	ChunkType chunkType;
	bool writable;

	Chunk(DirectorFile *d, ChunkType t) : dir(d), chunkType(t), writable(false) {}
	virtual ~Chunk() = default;
	virtual void read(Common::ReadStream &stream) = 0;
	virtual size_t size() { return 0; }
	virtual void write(Common::WriteStream&) {}
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct ListChunk : Chunk {
	uint32_t dataOffset;
	uint16_t offsetTableLen;
	std::vector<uint32_t> offsetTable;
	uint32_t itemsLen;
	Common::Endianness itemEndianness;
	std::vector<Common::BufferView> items;

	ListChunk(DirectorFile *m, ChunkType t) : Chunk(m, t) {}
	virtual void read(Common::ReadStream &stream);
	virtual void readHeader(Common::ReadStream &stream);
	void readOffsetTable(Common::ReadStream &stream);
	void readItems(Common::ReadStream &stream);

	std::string readString(uint16_t index);
	std::string readPascalString(uint16_t index);
	uint16_t readUint16(uint16_t index);
	uint32_t readUint32(uint16_t index);

	void updateOffsets();

	virtual size_t size();
	virtual size_t headerSize();
	size_t offsetTableSize();
	size_t itemsSize();
	virtual size_t itemSize(uint16_t index);

	virtual void write(Common::WriteStream &stream);
	virtual void writeHeader(Common::WriteStream &stream);
	void writeOffsetTable(Common::WriteStream &stream);
	void writeItems(Common::WriteStream &stream);
	virtual void writeItem(Common::WriteStream &stream, uint16_t index);
};

struct CastChunk : Chunk {
	std::vector<int32_t> memberIDs;
	std::string name;
	std::map<uint16_t, std::shared_ptr<CastMemberChunk>> members;
	std::shared_ptr<ScriptContextChunk> lctx;

	CastChunk(DirectorFile *m) : Chunk(m, kCastChunk) {}
	virtual ~CastChunk() = default;
	virtual void read(Common::ReadStream &stream);
	void populate(const std::string &castName, int32_t id, uint16_t minMember);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct CastListChunk : ListChunk {
	uint16_t unk0;
	uint16_t castCount;
	uint16_t itemsPerCast;
	uint16_t unk1;
	std::vector<CastListEntry> entries;

	CastListChunk(DirectorFile *m) : ListChunk(m, kCastListChunk) {}
	virtual ~CastListChunk() = default;
	virtual void read(Common::ReadStream &stream);
	virtual void readHeader(Common::ReadStream &stream);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct CastMemberChunk : Chunk {
	MemberType type;
	uint32_t infoLen;
	uint32_t specificDataLen;
	std::shared_ptr<CastInfoChunk> info;
	Common::BufferView specificData;
	std::unique_ptr<CastMember> member;
	bool hasFlags1;
	uint8_t flags1;

	uint16_t id;
	ScriptChunk *script;

	CastMemberChunk(DirectorFile *m) : Chunk(m, kCastMemberChunk), id(0), script(nullptr) {
		writable = true;
	}
	virtual ~CastMemberChunk() = default;
	virtual void read(Common::ReadStream &stream);
	virtual size_t size();
	virtual void write(Common::WriteStream &stream);
	virtual void writeJSON(Common::JSONWriter &json) const;

	uint32_t getScriptID() const;
	std::string getScriptText() const;
	void setScriptText(std::string val);
	std::string getName() const;
};

struct CastInfoChunk : ListChunk {
	uint32_t unk1;
	uint32_t unk2;
	uint32_t flags;
	uint32_t scriptId;

	std::string scriptSrcText;
	std::string name;
	// cProp02;
	// cProp03;
	// std::string comment;
	// cProp05;
	// cProp06;
	// cProp07;
	// cProp08;
	// xtraGUID;
	// cProp10;
	// cProp11;
	// cProp12;
	// cProp13;
	// cProp14;
	// cProp15;
	// std::string fileFormatID;
	// uint32_t created;
	// uint32_t modified;
	// cProp19;
	// cProp20;
	// imageCompression;

	CastInfoChunk(DirectorFile *m) : ListChunk(m, kCastInfoChunk) {
		writable = true;
	}
	virtual ~CastInfoChunk() = default;
	virtual void read(Common::ReadStream &stream);
	virtual void readHeader(Common::ReadStream &stream);
	virtual size_t headerSize();
	virtual void writeHeader(Common::WriteStream &stream);
	virtual size_t itemSize(uint16_t index);
	virtual void writeItem(Common::WriteStream &stream, uint16_t index);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct ConfigChunk : Chunk {
	/*  0 */ int16_t len;
	/*  2 */ int16_t fileVersion;
	/*  4 */ int16_t movieTop;
	/*  6 */ int16_t movieLeft;
	/*  8 */ int16_t movieBottom;
	/* 10 */ int16_t movieRight;
	/* 12 */ int16_t minMember;
	/* 14 */ int16_t maxMember;
	/* 16 */ int8_t field9;
	/* 17 */ int8_t field10;

	// Director 6 and below
		/* 18 */ int16_t preD7field11;
	// Director 7 and above
		/* 18 */ uint8_t D7stageColorG;
		/* 19 */ uint8_t D7stageColorB;

	/* 20 */ int16_t commentFont;
	/* 22 */ int16_t commentSize;
	/* 24 */ uint16_t commentStyle;

	// Director 6 and below
		/* 26 */ int16_t preD7stageColor;
	// Director 7 and above
		/* 26 */ uint8_t D7stageColorIsRGB;
		/* 27 */ uint8_t D7stageColorR;

	/* 28 */ int16_t bitDepth;
	/* 30 */ uint8_t field17;
	/* 31 */ uint8_t field18;
	/* 32 */ int32_t field19;
	/* 36 */ int16_t directorVersion;
	/* 38 */ int16_t field21;
	/* 40 */ int32_t field22;
	/* 44 */ int32_t field23;
	/* 48 */ int32_t field24;
	/* 52 */ int8_t field25;
	/* 53 */ uint8_t field26;
	/* 54 */ int16_t frameRate;
	/* 56 */ int16_t platform;
	/* 58 */ int16_t protection;
	/* 60 */ int32_t field29;
	/* 64 */ uint32_t checksum;
	/* 68 */ Common::BufferView remnants;
	ConfigChunk(DirectorFile *m) : Chunk(m, kConfigChunk) {
		writable = true;
	}
	virtual ~ConfigChunk() = default;
	virtual void read(Common::ReadStream &stream);
	virtual size_t size();
	virtual void write(Common::WriteStream &stream);
	uint32_t computeChecksum();
	void unprotect();
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct InitialMapChunk : Chunk {
	uint32_t version; // always 1
	uint32_t mmapOffset;
	uint32_t directorVersion;
	uint32_t unused1;
	uint32_t unused2;
	uint32_t unused3;

	InitialMapChunk(DirectorFile *m) : Chunk(m, kInitialMapChunk) {
		writable = true;
	}
	virtual ~InitialMapChunk() = default;
	virtual void read(Common::ReadStream &stream);
	virtual size_t size();
	virtual void write(Common::WriteStream &stream);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct KeyTableChunk : Chunk {
	uint16_t entrySize; // Should always be 12 (3 uint32's)
	uint16_t entrySize2;
	uint32_t entryCount;
	uint32_t usedCount;
	std::vector<KeyTableEntry> entries;

	KeyTableChunk(DirectorFile *m) : Chunk(m, kKeyTableChunk) {}
	virtual ~KeyTableChunk() = default;
	virtual void read(Common::ReadStream &stream);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct MemoryMapChunk : Chunk {
	int16_t headerLength; // should be 24
	int16_t entryLength; // should be 20
	int32_t chunkCountMax;
	int32_t chunkCountUsed;
	int32_t junkHead;
	int32_t junkHead2;
	int32_t freeHead;
	std::vector<MemoryMapEntry> mapArray;

	MemoryMapChunk(DirectorFile *m) : Chunk(m, kMemoryMapChunk) {
		writable = true;
	}
	virtual ~MemoryMapChunk() = default;
	virtual void read(Common::ReadStream &stream);
	virtual size_t size();
	virtual void write(Common::WriteStream &stream);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct ScriptChunk : Chunk {
	/*  8 */ uint32_t totalLength;
	/* 12 */ uint32_t totalLength2;
	/* 16 */ uint16_t headerLength;
	/* 18 */ uint16_t scriptNumber;

	/* 38 */ uint32_t scriptFlags;

	/* 50 */ uint16_t handlerVectorsCount;
	/* 52 */ uint32_t handlerVectorsOffset;
	/* 56 */ uint32_t handlerVectorsSize;
	/* 60 */ uint16_t propertiesCount;
	/* 62 */ uint32_t propertiesOffset;
	/* 66 */ uint16_t globalsCount;
	/* 68 */ uint32_t globalsOffset;
	/* 72 */ uint16_t handlersCount;
	/* 74 */ uint32_t handlersOffset;
	/* 78 */ uint16_t literalsCount;
	/* 80 */ uint32_t literalsOffset;
	/* 84 */ uint32_t literalsDataCount;
	/* 88 */ uint32_t literalsDataOffset;

	std::vector<int16_t> propertyNameIDs;
	std::vector<int16_t> globalNameIDs;

	std::vector<std::string> propertyNames;
	std::vector<std::string> globalNames;
	std::vector<std::unique_ptr<Handler>> handlers;
	std::vector<LiteralStore> literals;
	ScriptContextChunk *context;

	CastMemberChunk *member;

	ScriptChunk(DirectorFile *m);
	virtual ~ScriptChunk();
	virtual void read(Common::ReadStream &stream);
	std::vector<int16_t> readVarnamesTable(Common::ReadStream &stream, uint16_t count, uint32_t offset);
	std::string getName(int id);
	void setContext(ScriptContextChunk *ctx);
	void translate();
	std::string varDeclarations();
	std::string scriptText();
	std::string bytecodeText();
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct ScriptContextChunk : Chunk {
	int32_t unknown0;
	int32_t unknown1;
	uint32_t entryCount;
	uint32_t entryCount2;
	uint16_t entriesOffset;
	int16_t unknown2;
	int32_t unknown3;
	int32_t unknown4;
	int32_t unknown5;
	int32_t lnamSectionID;
	uint16_t validCount;
	uint16_t flags;
	int16_t freePointer;

	std::shared_ptr<ScriptNamesChunk> lnam;
	std::vector<ScriptContextMapEntry> sectionMap;
	std::map<uint32_t, std::shared_ptr<ScriptChunk>> scripts;

	ScriptContextChunk(DirectorFile *m) : Chunk(m, kScriptContextChunk) {}
	virtual ~ScriptContextChunk() = default;
	virtual void read(Common::ReadStream &stream);
	std::string getName(int id);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

struct ScriptNamesChunk : Chunk {
	int32_t unknown0;
	int32_t unknown1;
	uint32_t len1;
	uint32_t len2;
	uint16_t namesOffset;
	uint16_t namesCount;
	std::vector<std::string> names;

	ScriptNamesChunk(DirectorFile *m) : Chunk(m, kScriptNamesChunk) {}
	virtual ~ScriptNamesChunk() = default;
	virtual void read(Common::ReadStream &stream);
	std::string getName(int id);
	virtual void writeJSON(Common::JSONWriter &json) const;
};

} // namespace Director

#endif // DIRECTOR_CHUNK_H
