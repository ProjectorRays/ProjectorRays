import {
  Chunk,
  Meta,
  InitialMap,
  MemoryMap,
  ScriptContext,
  ScriptNames,
  Script,
  UnimplementedChunk
} from "./chunk";
import {InvalidDirectorFileError, PathTooNewError} from "./errors";
import {DataStream} from "./DataStream"

/* Movie */

export class Movie {
  chunkArrays: Map<string, Chunk[]>;
  chunkMap: Chunk[];
  _chunkBuffers: Map<Chunk, Buffer>;

  read(buffer: Buffer) {
    this.chunkArrays = new Map();
    this.chunkMap = [];
    this._chunkBuffers = new Map();

    var dataStream = new DataStream(buffer);
    dataStream.endianness = DataStream.BIG_ENDIAN; // we set this properly when we create the RIFX chunk
    this.lookupMmap(dataStream);
    this.linkScripts();
  }

  lookupMmap(dataStream: DataStream) {
    // at the beginning of the file, we need to break some of the typical rules. We don't know names, lengths and offsets yet.

    // valid length is undefined because we have not yet reached mmap
    // however, it will be filled automatically in chunk's constructor
    let meta: Meta = this.readChunk(dataStream, "RIFX");
    // we can only open DIR or DXR
    // we'll read Movie from dataStream because Movie is an exception to the normal rules
    if (meta.codec != "MV93") {
      throw new PathTooNewError("Codec " + meta.codec + " unsupported.");
    }
    this.addChunk("RIFX", meta);

    // the next chunk should be imap
    // this HAS to be dataStream for the OFFSET check to be correct
    // we will continue to use it because in this implementation RIFX doesn't contain it
    let imap: InitialMap = this.readChunk(dataStream, "imap", null, 12);
    this.addChunk("imap", imap);

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
      let mmap: MemoryMap = this.readChunk(dataStream, "mmap", null, memoryMapOffset);
      // add chunks in the mmap to the chunkArrays HERE
      // make sure to account for chunks with existing names, lengths and offsets
      for (let i = 0, l = mmap.mapArray.length; i < l; i++) {
        let mapEntry = mmap.mapArray[i];
        if (mapEntry.name != "mmap") {
          dataStream.seek(mapEntry.offset);
          let chunk = this.readChunk(dataStream, mapEntry.name, mapEntry.len, mapEntry.offset, mapEntry.padding, mapEntry.unknown0, mapEntry.link);
          this.addChunk(mapEntry.name, chunk);
          this.chunkMap[mapEntry.index] = chunk;
        }
      }
      this.addChunk("mmap", mmap);
    }
  }

  addChunk(name: string, chunk: Chunk) {
    let chunkArray: Chunk[] = this.chunkArrays.get(name);
    if (!chunkArray) {
      chunkArray = [];
      this.chunkArrays.set(name, chunkArray);
    }
    chunkArray.push(chunk);
  }

  linkScripts() {
    let ctxArray = this.chunkArrays.get("LctX") as ScriptContext[];
    if (ctxArray) {
      for (let context of ctxArray) {
        let scriptNames = this.chunkMap[context.lnamSectionID] as ScriptNames;
        context.scriptNames = scriptNames;
        for (let section of context.sectionMap) {
          if (section.sectionID > -1) {
            let script = this.chunkMap[section.sectionID] as Script;
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

    let result;
    switch (name) {
      case "RIFX":
        result = new Meta();
        result.read(chunkDataStream);
        break;
      case "imap":
        result = new InitialMap();
        result.read(chunkDataStream);
        break;
      case "mmap":
        result = new MemoryMap();
        result.read(chunkDataStream);
        break;
      case "LctX":
        result = new ScriptContext();
        result.read(chunkDataStream);
        break;
      case "Lnam":
        result = new ScriptNames();
        result.read(chunkDataStream);
        break;
      case "Lscr":
        result = new Script();
        result.read(chunkDataStream);
        break;
      default:
        result = new UnimplementedChunk(name);
    }

    this._chunkBuffers.set(result, chunkBuffer);

    return result;
  }
}
