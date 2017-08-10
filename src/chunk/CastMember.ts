import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";
import {CastMemberType} from "../lib";

/* CastMember */

export class CastMember implements Chunk {
  fourCC = "CASt";

  type: CastMemberType;
  specificDataOffset: number;
  specificDataLen: number;
  skipSize: number;
  unknownData: Buffer;
  offsetTableLen: number;
  offsetTable: number[];
  finalDataLen: number;
  dataOffset: number;

  scriptSrcText: string;
  name: string;
  cProp02: Buffer;
  cProp03: Buffer;
  comment: string;
  cProp05: Buffer;
  cProp06: Buffer;
  cProp07: Buffer;
  cProp08: Buffer;
  xtraGUID: Buffer;
  cProp10: Buffer;
  cProp11: Buffer;
  cProp12: Buffer;
  cProp13: Buffer;
  cProp14: Buffer;
  cProp15: Buffer;
  fileFormatID: String;
  created: number;
  modified: number;
  cProp19: Buffer;
  cProp20: Buffer;
  imageCompression: Buffer;

  scriptNumber: number;

  read(dataStream: DataStream) {
    dataStream.endianness = DataStream.BIG_ENDIAN;

    this.type = dataStream.readUint32();
    this.specificDataOffset = dataStream.readUint32();
    this.specificDataLen = dataStream.readUint32();
    this.skipSize = dataStream.readUint32();
    this.unknownData = dataStream.readBytes(this.skipSize - 4);
    this.offsetTableLen = dataStream.readUint16();
    this.offsetTable = [];
    for (let i = 0, l = this.offsetTableLen; i < l; i++) {
      this.offsetTable[i] = dataStream.readUint32();
    }
    this.finalDataLen = dataStream.readUint32();
    this.dataOffset = dataStream.position;

    this.scriptSrcText = this.readStringProperty(dataStream, 0);
    this.name = this.readPrefixedStringProperty(dataStream, 1);
    this.cProp02 = this.readProperty(dataStream, 2);
    this.cProp03 = this.readProperty(dataStream, 3);
    this.comment = this.readStringProperty(dataStream, 4);
    this.cProp05 = this.readProperty(dataStream, 5);
    this.cProp06 = this.readProperty(dataStream, 6);
    this.cProp07 = this.readProperty(dataStream, 7);
    this.cProp08 = this.readProperty(dataStream, 8);
    this.xtraGUID = this.readProperty(dataStream, 9);
    this.cProp10 = this.readProperty(dataStream, 10);
    this.cProp11 = this.readProperty(dataStream, 11);
    this.cProp12 = this.readProperty(dataStream, 12);
    this.cProp13 = this.readProperty(dataStream, 13);
    this.cProp14 = this.readProperty(dataStream, 14);
    this.cProp15 = this.readProperty(dataStream, 15);
    this.fileFormatID = this.readStringProperty(dataStream, 16);
    this.created = this.readUint32Property(dataStream, 17);
    this.modified = this.readUint32Property(dataStream, 18);
    this.cProp19 = this.readProperty(dataStream, 19);
    this.cProp20 = this.readProperty(dataStream, 20);
    this.imageCompression = this.readProperty(dataStream, 21);

    if (this.type === CastMemberType.script) {
      this.scriptNumber = this.unknownData.readUInt16BE(14) - 1;
    }
  }

  readProperty(dataStream, index: number) {
    if (index >= this.offsetTable.length - 1) return null;

    let offset = this.offsetTable[index];
    let nextOffset = index < this.offsetTableLen - 1 ? this.offsetTable[index + 1] : this.specificDataOffset;
    let length = nextOffset - offset;
    dataStream.seek(this.dataOffset + offset);
    return dataStream.readBytes(length);
  }

  readStringProperty(dataStream, index: number) {
    if (index >= this.offsetTable.length - 1) return null;

    let offset = this.offsetTable[index];
    let nextOffset = index < this.offsetTableLen - 1 ? this.offsetTable[index + 1] : this.specificDataOffset;
    let length = nextOffset - offset;
    dataStream.seek(this.dataOffset + offset);
    return dataStream.readString(length);
  }

  readPrefixedStringProperty(dataStream, index: number) {
    if (index >= this.offsetTable.length - 1) return null;

    let offset = this.offsetTable[index];
    dataStream.seek(this.dataOffset + offset);
    let length = dataStream.readUint8();
    return dataStream.readString(length);
  }

  readUint32Property(dataStream, index: number) {
    if (index >= this.offsetTable.length - 1) return null;

    let offset = this.offsetTable[index];
    dataStream.seek(this.dataOffset + offset);
    return dataStream.readUint32();
  }
}
