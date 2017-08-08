import {Subchunk} from "../Subchunk";
import {DataStream} from "../../DataStream";

/* CastKeyEntry */

export class CastKeyEntry implements Subchunk {
  sectionID: number;
  castID: number;
  fourCC: string;

  read(dataStream: DataStream) {
    this.sectionID = dataStream.readUint32();
    this.castID = dataStream.readUint32();
    this.fourCC = dataStream.readFourCC();
  }
}
