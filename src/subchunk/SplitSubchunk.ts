import {DataStream} from "../DataStream";

/* SplitSubchunk */

export interface SplitSubchunk {
  readRecord(dataStream: DataStream);
  readData(dataStream: DataStream, startOffset?: number);
}
