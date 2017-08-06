import {Script} from "../../chunk/Script";
import {SplitSubchunk} from "../SplitSubchunk";
import {DataStream} from "../../DataStream";

/* Literal */

export class Literal implements SplitSubchunk {
  type: number;
  offset: number;
  length: number;
  value: string | number;

  constructor() {}

  readRecord(dataStream: DataStream) {
    this.type = dataStream.readUint32();
    this.offset = dataStream.readUint32();
  }

  readData(dataStream: DataStream, startOffset: number) {
    if (this.type === 4) {
      this.value = this.offset;
    } else {
      dataStream.seek(startOffset + this.offset);
      this.length = dataStream.readUint32();
      if (this.type === 1) {
        this.value = dataStream.readString(this.length - 1);
      } else if (this.type === 9) {
        this.value = dataStream.readDouble();
      }
    }
  }
}
