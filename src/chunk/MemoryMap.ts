import {Chunk} from "./Chunk";
import {MemoryMapEntry} from "../subchunk/memorymap";
import {DataStream} from "../DataStream";

/* MemoryMap */

export class MemoryMap implements Chunk {
  name = "mmap";

  unknown0: number;
  unknown1: number;
  chunkCountMax: number;
  chunkCountUsed: number;
  junkPointer: number;
  unknown2: number;
  freePointer: number;
  mapArray: MemoryMapEntry[];

  read(dataStream: DataStream) {
    this.unknown0 = dataStream.readUint16();
    this.unknown1 = dataStream.readUint16();
    // possible one of the unknown mmap entries determines why an unused item is there?
    // it also seems code comments can be inserted after mmap after chunkCount is over, it may warrant investigation
    this.chunkCountMax = dataStream.readInt32();
    this.chunkCountUsed = dataStream.readInt32();
    this.junkPointer = dataStream.readInt32();
    this.unknown2 = dataStream.readInt32();
    this.freePointer = dataStream.readInt32();
    this.mapArray = [];
    // seems chunkCountUsed is used here, so what is chunkCount for?
    // EDIT: chunkCountMax is maximum allowed chunks before new mmap created!
    var entry;
    for (var i = 0, len = this.chunkCountUsed; i < len; i++) {
      // don't actually generate new chunk objects here, just read in data
      let entry = new MemoryMapEntry(i);
      entry.read(dataStream);
      // we don't care about free or junk chunks
      if (entry.name !== "free" && entry.name !== "junk") {
        this.mapArray.push(entry);
      }
    }
  }
}
