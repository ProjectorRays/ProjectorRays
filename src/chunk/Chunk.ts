import {Movie} from "../Movie";
import {DataStream} from "../DataStream";

/* Chunk */

export interface Chunk {
  name: string;
  read(dataStream: DataStream);
}
