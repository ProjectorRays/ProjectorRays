import {Chunk} from "./Chunk";
import {ScriptNames} from "./ScriptNames";
import {Script} from "./Script";
import {ScriptContextMapEntry} from "../subchunk/scriptcontext";
import {DataStream} from "../DataStream";

/* ScriptContext */

export class ScriptContext implements Chunk {
  name = "LctX";

  unknown0: number;
  unknown1: number;
  entryCount: number;
  entryCount2: number;
  entriesOffset: number;
  unknown2: number;
  unknown3: number;
  unknown4: number;
  unknown5: number;
  lnamSectionID: number;
  validCount: number;
  flags: number;
  freePointer: number;
  sectionMap: ScriptContextMapEntry[];

  scriptNames: ScriptNames;
  scripts: Script[] = [];

  read(dataStream: DataStream) {
    // Lingo scripts are always big endian regardless of file endianness
    dataStream.endianness = DataStream.BIG_ENDIAN;

    this.unknown0 = dataStream.readInt32();
    this.unknown1 = dataStream.readInt32();
    this.entryCount = dataStream.readUint32();
    this.entryCount2 = dataStream.readUint32();
    this.entriesOffset = dataStream.readUint16();
    this.unknown2 = dataStream.readInt16();
    this.unknown3 = dataStream.readInt32();
    this.unknown4 = dataStream.readInt32();
    this.unknown5 = dataStream.readInt32();
    this.lnamSectionID = dataStream.readInt32();
    this.validCount = dataStream.readUint16();
    this.flags = dataStream.readUint16();
    this.freePointer = dataStream.readInt16();

    dataStream.seek(this.entriesOffset);
    this.sectionMap = [];
    for (let i = 0, l = this.entryCount; i < l; i++) {
      let section = new ScriptContextMapEntry();
      section.read(dataStream);
      this.sectionMap.push(section);
    }
  }
}
