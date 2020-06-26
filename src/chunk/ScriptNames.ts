import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";

/* ScriptNames */

export class ScriptNames implements Chunk {
    fourCC = "Lnam";

    unknown0: number;
    unknown1: number;
    len1: number;
    len2: number;
    namesOffset: number;
    nameCount: number;
    names: string[];

    read(dataStream: DataStream) {
        // Lingo scripts are always big endian regardless of file endianness
        dataStream.endianness = DataStream.BIG_ENDIAN;

        this.unknown0 = dataStream.readInt32();
        this.unknown1 = dataStream.readInt32();
        this.len1 = dataStream.readUint32();
        this.len2 = dataStream.readUint32();
        this.namesOffset = dataStream.readUint16();
        this.nameCount = dataStream.readUint16();

        dataStream.seek(this.namesOffset);
        this.names = [];
        for (let i = 0, l = this.nameCount; i < l; i++) {
            let length = dataStream.readUint8();
            let name = dataStream.readString(length);
            this.names.push(name);
        }
    }
}
