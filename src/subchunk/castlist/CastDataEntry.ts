import {Subchunk} from "../Subchunk";
import {DataStream} from "../../DataStream";

/* CastDataEntry */

export class CastDataEntry implements Subchunk {
    name: string;
    filePath: string;
    preloadSettings: number;
    storageType: number;
    membersCount: number;
    id: number;

    read(dataStream: DataStream) {
        let nameLength = dataStream.readUint8();
        this.name = dataStream.readString(nameLength);

        let filePathLength = dataStream.readUint8();
        this.filePath = dataStream.readString(filePathLength);

        this.preloadSettings = dataStream.readUint16();

        this.storageType = dataStream.readUint16();
        this.membersCount = dataStream.readUint16();
        this.id = dataStream.readUint32();
    }
}
