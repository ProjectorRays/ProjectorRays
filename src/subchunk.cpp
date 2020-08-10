#include "lingo.h"
#include "stream.h"
#include "subchunk.h"

namespace ProjectorRays {

/* CastKeyEntry */

void CastKeyEntry::read(ReadStream &stream) {
    sectionID = stream.readUint32();
    castID = stream.readUint32();
    fourCC = stream.readUint32();
}

/* MemoryMapEntry */

void MemoryMapEntry::read(ReadStream &stream) {
    fourCC = stream.readUint32();
    len = stream.readUint32();
    offset = stream.readUint32();
    padding = stream.readInt16();
    unknown0 = stream.readInt16();
    link = stream.readInt32();
}

/* ScriptContextMapEntry */

void ScriptContextMapEntry::read(ReadStream &stream) {
    unknown0 = stream.readInt32();
    sectionID = stream.readInt32();
    unknown1 = stream.readUint16();
    unknown2 = stream.readUint16();
}

/* LiteralStore */

void LiteralStore::readRecord(ReadStream &stream) {
    type = static_cast<LiteralType>(stream.readUint32());
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
