import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";

/* Sord */

export class Sord {
  fourCC = "Sord";

  entryCount: number;
  entries: number[]

  read(dataStream: DataStream) {
    dataStream.endianness = DataStream.BIG_ENDIAN;

    dataStream.seek(8);
    this.entryCount = dataStream.readUint32();
    dataStream.skip(8);
    this.entries = [];
    for (let i = 0, l = this.entryCount; i < l; i++) {
      this.entries[i] = dataStream.readUint32();
    }
  }
}
