import {Subchunk} from "../Subchunk";
import {DataStream} from "../../DataStream";

/* MemoryMapEntry */

export class MemoryMapEntry implements Subchunk {
    name: string;
    len: number;
    offset: number;
    padding: number;
    unknown0: number;
    link: number;

    constructor(public index: number) {}

    read(dataStream: DataStream) {
        this.name = dataStream.readFourCC();
        this.len = dataStream.readUint32();
        this.offset = dataStream.readUint32();
        this.padding = dataStream.readInt16();
        this.unknown0 = dataStream.readInt16();
        this.link = dataStream.readInt32();
    }
}
