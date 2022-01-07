/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2022 Deborah Servilla
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DIRECTOR_CHUNK_H
#define DIRECTOR_CHUNK_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "common/stream.h"
#include "director/castmember.h"
#include "director/subchunk.h"

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
	virtual void write(Common::WriteStream &stream) {}
};
void to_json(ordered_json &j, const Chunk &c);

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
};
void to_json(ordered_json &j, const CastChunk &c);

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
};
void to_json(ordered_json &j, const CastListChunk &c);

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
};
void to_json(ordered_json &j, const CastMemberChunk &c);

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
};
void to_json(ordered_json &j, const CastInfoChunk &c);

struct ConfigChunk : Chunk {
	/*  0 */ uint16_t len;
	/*  2 */ uint16_t fileVersion;
	/*  4 */ int16_t movieTop;
	/*  6 */ int16_t movieLeft;
	/*  8 */ int16_t movieBottom;
	/* 10 */ int16_t movieRight;
	/* 12 */ uint16_t minMember;
	/* 14 */ uint16_t maxMember;
	/* 16 */ uint8_t field9;
	/* 17 */ uint8_t field10;
	/* 18 */ int16_t field11;
	/* 20 */ int16_t commentFont;
	/* 22 */ int16_t commentSize;
	/* 24 */ uint16_t commentStyle;
	/* 26 */ int16_t stageColor;
	/* 28 */ int16_t bitDepth;
	/* 30 */ uint8_t field17;
	/* 31 */ uint8_t field18;
	/* 32 */ int32_t field19;
	/* 36 */ int16_t directorVersion;
	/* 38 */ int16_t field21;
	/* 40 */ int32_t field22;
	/* 44 */ int32_t field23;
	/* 48 */ int32_t field24;
	/* 52 */ uint8_t field25;
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
};
void to_json(ordered_json &j, const ConfigChunk &c);

struct InitialMapChunk : Chunk {
	uint32_t one; // always 1
	uint32_t mmapOffset;
	uint32_t version;
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
};
void to_json(ordered_json &j, const InitialMapChunk &c);

struct KeyTableChunk : Chunk {
	uint16_t entrySize; // Should always be 12 (3 uint32's)
	uint16_t entrySize2;
	uint32_t entryCount;
	uint32_t usedCount;
	std::vector<KeyTableEntry> entries;

	KeyTableChunk(DirectorFile *m) : Chunk(m, kKeyTableChunk) {}
	virtual ~KeyTableChunk() = default;
	virtual void read(Common::ReadStream &stream);
};
void to_json(ordered_json &j, const KeyTableChunk &c);

struct MemoryMapChunk : Chunk {
	uint16_t headerLength; // should be 24
	uint16_t entryLength; // should be 20
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
};
void to_json(ordered_json &j, const MemoryMapChunk &c);

struct ScriptChunk : Chunk {
	uint32_t totalLength;
	uint32_t totalLength2;
	uint16_t headerLength;
	uint16_t scriptNumber;
	uint16_t scriptBehavior;

	uint16_t handlerVectorsCount;
	uint32_t handlerVectorsOffset;
	uint32_t handlerVectorsSize;
	uint16_t propertiesCount;
	uint32_t propertiesOffset;
	uint16_t globalsCount;
	uint32_t globalsOffset;
	uint16_t handlersCount;
	uint32_t handlersOffset;
	uint16_t literalsCount;
	uint32_t literalsOffset;
	uint32_t literalsDataCount;
	uint32_t literalsDataOffset;

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
};
void to_json(ordered_json &j, const ScriptChunk &c);

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
};
void to_json(ordered_json &j, const ScriptContextChunk &c);

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
};
void to_json(ordered_json &j, const ScriptNamesChunk &c);

}

#endif
