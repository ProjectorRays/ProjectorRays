import * as Chunk from "./chunk";
import {CastDataEntry} from "./subchunk/castlist";
import {CastMemberType} from "./lib";

/* Cast */

export class Cast {
    name: string;
    filePath: string;
    preloadSettings: number;
    storageType: number;
    id: number;
    id1: number;
    id2: number;

    chunkArrays: Map<string, Chunk.Chunk[]> = new Map();
    members: Chunk.CastMember[] = [];
    scripts: Chunk.CastMember[] = [];

    readDataEntry(entry: CastDataEntry) {
        this.name = entry.name;
        this.filePath = entry.filePath;
        this.preloadSettings = entry.preloadSettings;
        this.storageType = entry.storageType;
        this.id = entry.id;
        this.id1 = entry.id & 0xff;
        this.id2 = (entry.id >> 16) & 0xff;
    }

    readKey(key: Chunk.CastKey, chunkMap: Chunk.Chunk[]) {
        for (let entry of key.entries) {
            if (entry.castID === this.id) {
                let chunkArray = this.chunkArrays.get(entry.fourCC);
                if (!chunkArray) {
                    chunkArray = [];
                    this.chunkArrays.set(entry.fourCC, chunkArray);
                }
                let chunk = chunkMap[entry.sectionID];
                if (chunkArray.indexOf(chunk) === -1) {
                    chunkArray.push(chunk);
                }
            }
        }
        let associationsArray = this.chunkArrays.get("CAS*");
        if (associationsArray) {
            let associations = associationsArray[0] as Chunk.CastAssociations;
            this.readAssociations(associations, chunkMap);
        }
    }

    readAssociations(associations: Chunk.CastAssociations, chunkMap: Chunk.Chunk[]) {
        for (let i = 0, l = associations.entries.length; i < l; i++) {
            let castMember = chunkMap[associations.entries[i]] as Chunk.CastMember;
            this.members[i] = castMember;
            if (castMember.type === CastMemberType.script) {
                this.scripts[castMember.scriptNumber] = castMember;
            }
        }
    }
}
