import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";
import {CastDataEntry} from "../subchunk/castlist";

/* CastList */

export class CastList implements Chunk {
    fourCC = "MCsL";

    unknown0: number;
    castCount: number;
    unknown1: number;
    arraySize: number;
    offsetTable: Buffer;
    castEntriesLength: number;
    entries: CastDataEntry[];

    read(dataStream: DataStream) {
        dataStream.endianness = DataStream.BIG_ENDIAN;

        this.unknown0 = dataStream.readUint32();
        this.castCount = dataStream.readUint32();
        this.unknown1 = dataStream.readUint16();
        this.arraySize = dataStream.readUint32();
        this.offsetTable = dataStream.readBytes(this.arraySize * 4);
        this.castEntriesLength = dataStream.readUint32();
        this.entries = [];
        for (let i = 0, l = this.castCount; i < l; i++) {
            let entry = new CastDataEntry();
            entry.read(dataStream);
            this.entries[i] = entry;
        }
    }
}
