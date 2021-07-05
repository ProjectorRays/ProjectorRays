#include <nlohmann/json.hpp>
using ordered_json = nlohmann::ordered_json;

#include "common/stream.h"
#include "director/lingo.h"
#include "director/subchunk.h"
#include "director/util.h"

namespace Director {

/* CastListEntry */

void to_json(ordered_json &j, const CastListEntry &c) {
    j["name"] = c.name;
    j["filePath"] = c.filePath;
    j["preloadSettings"] = c.preloadSettings;
    j["minMember"] = c.minMember;
    j["maxMember"] = c.maxMember;
    j["id"] = c.id;
}

/* MemoryMapEntry */

void MemoryMapEntry::read(Common::ReadStream &stream) {
    fourCC = stream.readUint32();
    len = stream.readUint32();
    offset = stream.readUint32();
    flags = stream.readInt16();
    unknown0 = stream.readInt16();
    next = stream.readInt32();
}

void to_json(ordered_json &j, const MemoryMapEntry &c) {
    j["fourCC"] = fourCCToString(c.fourCC);
    j["len"] = c.len;
    j["offset"] = c.offset;
    j["flags"] = c.flags;
    j["unknown0"] = c.unknown0;
    j["next"] = c.next;
}

/* ScriptContextMapEntry */

void ScriptContextMapEntry::read(Common::ReadStream &stream) {
    unknown0 = stream.readInt32();
    sectionID = stream.readInt32();
    unknown1 = stream.readUint16();
    unknown2 = stream.readUint16();
}

void to_json(ordered_json &j, const ScriptContextMapEntry &c) {
    j["unknown0"] = c.unknown0;
    j["sectionID"] = c.sectionID;
    j["unknown1"] = c.unknown1;
    j["unknown2"] = c.unknown2;
}

/* KeyTableEntry */

void KeyTableEntry::read(Common::ReadStream &stream) {
    sectionID = stream.readInt32();
    castID = stream.readInt32();
    fourCC = stream.readUint32();
}

void to_json(ordered_json &j, const KeyTableEntry &c) {
    j["sectionID"] = c.sectionID;
    j["castID"] = c.castID;
    j["fourCC"] = fourCCToString(c.fourCC);
}

/* LiteralStore */

void LiteralStore::readRecord(Common::ReadStream &stream, int version) {
    if (version >= 500)
        type = static_cast<LiteralType>(stream.readUint32());
    else
        type = static_cast<LiteralType>(stream.readUint16());
    offset = stream.readUint32();
}

void LiteralStore::readData(Common::ReadStream &stream, uint32_t startOffset) {
    if (type == kLiteralInt) {
        value = std::make_shared<Datum>((int)offset);
    } else {
        stream.seek(startOffset + offset);
        auto length = stream.readUint32();
        if (type == kLiteralString) {
            value = std::make_shared<Datum>(kDatumString, stream.readString(length - 1));
        } else if (type == kLiteralFloat) {
            double floatVal = 0.0;
            if (length == 8) {
                floatVal = stream.readDouble();
            } else if (length == 10) {
                floatVal = stream.readAppleFloat80();
            }
            value = std::make_shared<Datum>(floatVal);
        } else {
            value = std::make_shared<Datum>();
        }
    }
}

void to_json(ordered_json &j, const LiteralStore &c) {
    j["type"] = c.type;
    j["offset"] = c.offset;
    j["value"] = *c.value;
}

/* Rectangle */

void Rectangle::read(Common::ReadStream &stream) {
    top = stream.readUint16();
    left = stream.readUint16();
    bottom = stream.readUint16();
    right = stream.readUint16();
}

void to_json(ordered_json &j, const Rectangle &c) {
    j["top"] = c.top;
    j["left"] = c.left;
    j["bottom"] = c.bottom;
    j["right"] = c.right;
}

}
