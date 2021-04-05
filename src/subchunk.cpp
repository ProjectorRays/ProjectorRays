#include "lingo.h"
#include "stream.h"
#include "subchunk.h"

namespace ProjectorRays {

/* MemoryMapEntry */

void MemoryMapEntry::read(ReadStream &stream) {
    fourCC = stream.readUint32();
    len = stream.readUint32();
    offset = stream.readUint32();
    flags = stream.readInt16();
    unknown0 = stream.readInt16();
    next = stream.readInt32();
}

/* ScriptContextMapEntry */

void ScriptContextMapEntry::read(ReadStream &stream) {
    unknown0 = stream.readInt32();
    sectionID = stream.readInt32();
    unknown1 = stream.readUint16();
    unknown2 = stream.readUint16();
}

/* KeyTableEntry */

void KeyTableEntry::read(ReadStream &stream) {
    sectionID = stream.readInt32();
    castID = stream.readInt32();
    fourCC = stream.readUint32();
}

/* LiteralStore */

void LiteralStore::readRecord(ReadStream &stream, int version) {
    if (version >= 500)
        type = static_cast<LiteralType>(stream.readUint32());
    else
        type = static_cast<LiteralType>(stream.readUint16());
    offset = stream.readUint32();
}

void LiteralStore::readData(ReadStream &stream, uint32_t startOffset) {
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

/* Rectangle */

void Rectangle::read(ReadStream &stream) {
    top = stream.readUint16();
    left = stream.readUint16();
    bottom = stream.readUint16();
    right = stream.readUint16();
}

}
