import {DataStream} from "../DataStream";

/* Subchunk */

export interface Subchunk {
  read(dataStream: DataStream);
}
