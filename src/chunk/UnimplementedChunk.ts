import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";

/* UnimplementedChunk */

export class UnimplementedChunk implements Chunk {
  constructor(public name: string) {}

  read(dataStream: DataStream) {}
}
