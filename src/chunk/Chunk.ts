import {Movie} from "../Movie";
import {DataStream} from "../DataStream";

/* Chunk */

export interface Chunk {
  fourCC: string;
  read(dataStream: DataStream);
}
