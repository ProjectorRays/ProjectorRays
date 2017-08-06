import {Subchunk} from "../Subchunk";
import {DataStream} from "../../DataStream";

/* ScriptContextMapEntry */

export class ScriptContextMapEntry {
  unknown0: number;
  sectionID: number;
  unknown1: number;
  unknown2: number;

  read(dataStream: DataStream) {
    this.unknown0 = dataStream.readInt32();
    this.sectionID = dataStream.readInt32();
    this.unknown1 = dataStream.readUint16();
    this.unknown2 = dataStream.readUint16();
  }
}
