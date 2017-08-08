import {Chunk} from "./Chunk";
import {ScriptContext} from "./ScriptContext";
import {
  ScriptMapEntry,
  Handler,
  Literal
} from "../subchunk/script";
import {Stack} from "../lingo/Stack";
import {DataStream} from "../DataStream";

const MAP_ENTRIES = [
  "handler_vectors",
  "properties",
  "globals",
  "handlers",
  "literals",
  "literals_data"
];

/* Script */

export class Script implements Chunk {
  fourCC = "Lscr";

  totalLength: number;
  totalLength2: number;
  headerLength: number;
  scriptNumber: number;
  scriptBehaviour: number;
  map: Map<string, ScriptMapEntry>;
  propertyNameIDs: number[];
  globalNameIDs: number[];

  propertyNames: string[];
  globalNames: string[];
  handlers: Handler[];
  literals: Literal[];
  context: ScriptContext;

  stack: Stack;

  constructor() {
    this.stack = new Stack();
  }

  read(dataStream: DataStream) {
    dataStream.seek(8);
    // Lingo scripts are always big endian regardless of file endianness
    dataStream.endianness = DataStream.BIG_ENDIAN;
    this.totalLength = dataStream.readUint32();
    this.totalLength2 = dataStream.readUint32();
    this.headerLength = dataStream.readUint16();
    this.scriptNumber = dataStream.readUint16();
    dataStream.seek(38);
    this.scriptBehaviour = dataStream.readUint32();
    dataStream.seek(50);
    this.map = new Map();
    for (let name of MAP_ENTRIES) {
      let entry = new ScriptMapEntry(name);
      entry.read(dataStream);
      this.map.set(name, entry);
    }
    this.propertyNameIDs = this.readVarnamesTable(dataStream, this.map.get("properties").len, this.map.get("properties").offset);
    this.globalNameIDs = this.readVarnamesTable(dataStream, this.map.get("globals").len, this.map.get("globals").offset);

    dataStream.seek(this.map.get("handlers").offset);
    this.handlers = [];
    for (let i = 0, l = this.map.get("handlers").len; i < l; i++) {
      let handler = new Handler(this);
      handler.readRecord(dataStream);
      this.handlers.push(handler);
    }
    for (let handler of this.handlers) {
      handler.readData(dataStream);
    }

    dataStream.seek(this.map.get("literals").offset);
    this.literals = [];
    for (let i = 0, l = this.map.get("literals").len; i < l; i++) {
      let literal = new Literal();
      literal.readRecord(dataStream);
      this.literals.push(literal);
    }
    let literalsDataOffset = this.map.get("literals_data").offset;
    for (let literal of this.literals) {
      literal.readData(dataStream, literalsDataOffset);
    }
  }

  readVarnamesTable(dataStream, count, offset) {
    dataStream.seek(offset);
    let nameIDs = [];
    for (let i = 0; i < count; i++) {
      nameIDs.push(dataStream.readUint16());
    }
    return nameIDs;
  }

  readNames() {
    let nameList = this.context.scriptNames.names;
    this.propertyNames = this.propertyNameIDs.map(nameID => nameList[nameID]);
    this.globalNames = this.globalNameIDs.map(nameID => nameList[nameID]);
    for (let handler of this.handlers) {
      handler.readNames();
    }
  }

  translate() {
    for (let handler of this.handlers) {
      handler.translate();
    }
  }

  toString() {
    return this.handlers.map(handler => handler.ast.toString()).join("\n\n");
  }
}
