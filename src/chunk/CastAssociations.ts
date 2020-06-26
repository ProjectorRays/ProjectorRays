import {Chunk} from "./Chunk";
import {DataStream} from "../DataStream";

/* CastAssociations */

export class CastAssociations implements Chunk {
    fourCC = "CAS*";

    entries: number[];

    read(dataStream: DataStream) {
        dataStream.endianness = DataStream.BIG_ENDIAN;

        this.entries = [];
        while (!dataStream.endOfFile()) {
            let id = dataStream.readUint32();
            if (id > 0) {
                this.entries.push(id);
            }
        }
    }
}
