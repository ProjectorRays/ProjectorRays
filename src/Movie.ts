import * as Chunk from "./chunk";
import {InvalidDirectorFileError, PathTooNewError} from "./errors";
import {DataStream} from "./DataStream";
import {Cast} from "./Cast";

/* Movie */

export class Movie {
    chunkArrays: Map<string, Chunk.Chunk[]>;
    chunkMap: Chunk.Chunk[];
    _chunkBuffers: Map<Chunk.Chunk, Buffer>;

    castMap: Cast[];
    castMap1: Cast[];
    castMap2: Cast[];

    read(buffer: Buffer) {
        this.chunkArrays = new Map();
        this.chunkMap = [];
        this._chunkBuffers = new Map();
        this.castMap = [];
        this.castMap1 = [];
        this.castMap2 = [];

        var dataStream = new DataStream(buffer);
        dataStream.endianness = DataStream.BIG_ENDIAN; // we set this properly when we create the RIFX chunk
        this.lookupMmap(dataStream);
        this.createCasts();
        this.linkScripts();
    }

    lookupMmap(dataStream: DataStream) {
        // at the beginning of the file, we need to break some of the typical rules. We don't know names, lengths and offsets yet.

        // valid length is undefined because we have not yet reached mmap
        // however, it will be filled automatically in chunk's constructor
        let meta: Chunk.Meta = this.readChunk(dataStream, "RIFX");
        // we can only open DIR or DXR
        // we'll read Movie from dataStream because Movie is an exception to the normal rules
        if (meta.codec != "MV93") {
            throw new PathTooNewError("Codec " + meta.codec + " unsupported.");
        }

        // the next chunk should be imap
        // this HAS to be dataStream for the OFFSET check to be correct
        // we will continue to use it because in this implementation RIFX doesn't contain it
        let imap: Chunk.InitialMap = this.readChunk(dataStream, "imap", null, 12);

        // sanitize mmaps
        /*
        this.differenceImap = 0;
        if (imap.memoryMapArray[0] - 0x2C) {
            this.differenceImap = imap.memoryMapArray[0] - 0x2C;
            for (let i = 0, l = imap.memoryMapArray.length; i < l; i++) {
                imap.memoryMapArray[i] -= this.differenceImap;
            }
        }
        */

        for (let memoryMapOffset of imap.memoryMapArray) {
            dataStream.seek(memoryMapOffset);
            let mmap: Chunk.MemoryMap = this.readChunk(dataStream, "mmap", null, memoryMapOffset);
            // add chunks in the mmap to the chunkArrays HERE
            // make sure to account for chunks with existing names, lengths and offsets
            for (let i = 0, l = mmap.mapArray.length; i < l; i++) {
                let mapEntry = mmap.mapArray[i];
                let chunk;
                if (mapEntry.name === "RIFX") {
                    chunk = meta;
                } else if (mapEntry.name === "imap") {
                    chunk = imap;
                } else if (mapEntry.name === "mmap") {
                    chunk = mmap;
                } else {
                    dataStream.seek(mapEntry.offset);
                    chunk = this.readChunk(dataStream, mapEntry.name, mapEntry.len, mapEntry.offset, mapEntry.padding, mapEntry.unknown0, mapEntry.link);
                }
                this.addChunk(mapEntry.name, chunk);
                this.chunkMap[mapEntry.index] = chunk;
            }
        }
    }

    addChunk(name: string, chunk: Chunk.Chunk) {
        let chunkArray: Chunk.Chunk[] = this.chunkArrays.get(name);
        if (!chunkArray) {
            chunkArray = [];
            this.chunkArrays.set(name, chunkArray);
        }
        chunkArray.push(chunk);
    }

    createCasts() {
        let castList = this.chunkArrays.get("MCsL")[0] as Chunk.CastList;
        let castKey = this.chunkArrays.get("KEY*")[0] as Chunk.CastKey;
        for (let entry of castList.entries) {
            let cast = new Cast();
            cast.readDataEntry(entry);
            cast.readKey(castKey, this.chunkMap);
            this.castMap[cast.id] = cast;
            this.castMap1[cast.id1] = cast;
            this.castMap2[cast.id2] = cast;
        }
    }

    linkScripts() {
        let ctxArray = this.chunkArrays.get("LctX") as Chunk.ScriptContext[];
        if (ctxArray) {
            for (let context of ctxArray) {
                let scriptNames = this.chunkMap[context.lnamSectionID] as Chunk.ScriptNames;
                context.scriptNames = scriptNames;
                for (let section of context.sectionMap) {
                    if (section.sectionID > -1) {
                        let script = this.chunkMap[section.sectionID] as Chunk.Script;
                        script.context = context;
                        script.readNames();
                        script.translate();
                        context.scripts.push(script);
                    }
                }
            }
        }
    }

    readChunk(mainDataStream: any, name: string, len: number = null, offset: number = null, padding: number = null, unknown0: number = null, link: number = null) {
        // the offset is checked against, well, our offset
        let validOffset = mainDataStream.position;
        // check if this is the chunk we are expecting
        let validName = mainDataStream.readFourCC();
        if (name == "RIFX") {
            //if (validName.substring(0, 2) == "MZ") {
                // handle Projector HERE
            //}
            if (validName == "XFIR") {
                mainDataStream.endianness = DataStream.LITTLE_ENDIAN;
                validName = "RIFX";
            }
        }
        // check if it has the length the mmap table specifies
        let validLen = mainDataStream.readUint32();

        // use the valid values if mmap hasn't been read yet
        if (name == null) {
            name = validName;
        }
        if (len == null) {
            len = validLen;
        }
        if (offset == null) {
            offset = validOffset;
        }

        // padding can't be checked, so let's give it a default value if we don't yet know it
        if (padding == null) {
            // padding is usually zero
            if (name !== "free" && name !== "junk") {
                padding = 0;
            } else {
                padding = 12;
            }
        }

        // validate chunk
        if (name !== validName || len !== validLen || offset !== validOffset) {
            throw new InvalidDirectorFileError("At offset " + validOffset + ", expected '" + name + "' chunk with a length of " + len + " and offset of " + offset + " but found a '" + validName + "' chunk with a length of " + validLen + ".");
        }

        if (name === "RIFX") {
            // we're going to pretend RIFX is only 12 bytes long
            // this is because offsets are relative to the beginning of the file
            // whereas everywhere else they're relative to chunk start
            len = 4;
        }

        // copy the contents of the chunk to a new DataStream (minus name/length as that's not what offsets are usually relative to)
        let chunkBuffer = mainDataStream.readBytes(len);
        let chunkDataStream = new DataStream(chunkBuffer, mainDataStream.endianness);

        const chunks = {
            "RIFX": Chunk.Meta,
            "imap": Chunk.InitialMap,
            "mmap": Chunk.MemoryMap,
            "LctX": Chunk.ScriptContext,
            "Lnam": Chunk.ScriptNames,
            "Lscr": Chunk.Script,
            "MCsL": Chunk.CastList,
            "KEY*": Chunk.CastKey,
            "CAS*": Chunk.CastAssociations,
            "CASt": Chunk.CastMember
        };

        let result = chunks[name] ? new chunks[name]() : new Chunk.Unimplemented(name);
        result.read(chunkDataStream);

        this._chunkBuffers.set(result, chunkBuffer);

        return result;
    }
}
