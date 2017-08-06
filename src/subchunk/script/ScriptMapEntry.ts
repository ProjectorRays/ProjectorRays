import {Subchunk} from "../Subchunk";
import {DataStream} from "../../DataStream";

/* ScriptMapEntry */

export class ScriptMapEntry {
  len: number;
  offset: number;
  flags: number;

  constructor(public name: string) {}

  read(dataStream: DataStream) {
    if (this.name === "literals_data") {
      this.len = dataStream.readUint32();
    } else {
      this.len = dataStream.readUint16();
    }
    this.offset = dataStream.readUint32();
    if (this.name === "handler_vectors") {
      this.flags = dataStream.readUint32();
    }
  }
}
