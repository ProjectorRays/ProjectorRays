import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";
import {CastKeyEntry} from "../subchunk/castkey";

/* CastKey */

export class CastKey implements Chunk {
    fourCC = "KEY*";

    unknown0: number;
    unknown1: number;
    entryCount: number;
    unknown2: number;
    entries: CastKeyEntry[];

    read(dataStream: DataStream) {
        this.unknown0 = dataStream.readUint16();
        this.unknown1 = dataStream.readUint16();
        this.entryCount = dataStream.readUint32();
        this.unknown2 = dataStream.readUint32();

        this.entries = [];
        for (let i = 0, l = this.entryCount; i < l; i++) {
            let entry = new CastKeyEntry();
            entry.read(dataStream);
            if (entry.sectionID > 0) {
                this.entries.push(entry);
            }
        }
    }
}
