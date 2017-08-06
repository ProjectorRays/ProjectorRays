import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";

/* MetaChunk */

export class Meta implements Chunk {
  name = "RIFX";

  codec: string;

  read(dataStream: DataStream) {
    this.codec = dataStream.readFourCC();
  }
}
