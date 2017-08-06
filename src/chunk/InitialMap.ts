import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";

/* InitialMap */

export class InitialMap implements Chunk {
  name = "imap";

  memoryMapCount: number;
  memoryMapArray: number[];

  read(dataStream: DataStream) {
    this.memoryMapCount = dataStream.readUint32();
    this.memoryMapArray = [];
    for (let i = 0, l = this.memoryMapCount; i < l; i++) {
      this.memoryMapArray[i] = dataStream.readUint32();
    }
  }
}
