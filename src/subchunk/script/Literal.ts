import {Script} from "../../chunk/Script";
import {SplitSubchunk} from "../SplitSubchunk";
import {DataStream} from "../../DataStream";
import {LiteralType} from "../../lib";

/* Literal */

export class Literal implements SplitSubchunk {
    type: LiteralType;
    offset: number;
    length: number;
    value: string | number;

    constructor() {}

    readRecord(dataStream: DataStream) {
        this.type = dataStream.readUint32();
        this.offset = dataStream.readUint32();
    }

    readData(dataStream: DataStream, startOffset: number) {
        if (this.type === LiteralType.int) {
            this.value = this.offset;
        } else {
            dataStream.seek(startOffset + this.offset);
            this.length = dataStream.readUint32();
            if (this.type === LiteralType.string) {
                this.value = dataStream.readString(this.length - 1);
            } else if (this.type === LiteralType.float) {
                this.value = dataStream.readDouble();
            }
        }
    }
}
