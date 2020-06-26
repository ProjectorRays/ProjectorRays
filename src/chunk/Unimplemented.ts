import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";

/* Unimplemented */

export class Unimplemented implements Chunk {
    constructor(public fourCC: string) {}

    read(dataStream: DataStream) {}
}
