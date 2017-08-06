import {Handler} from "../subchunk/script/Handler";
import {Node} from "./AST";

/* Bytecode */  

export class Bytecode {
  opcode: string;
  translation: Node;

  constructor(public val: number, public obj: number, public objLength: number, public pos: number) {
    this.opcode = this.getOpcode(this.val);
  }

  getOpcode(val) {
    const oneByteCodes = {
      0x01: "ret",
      0x03: "pushint0",
      0x04: "mul",
      0x05: "add",
      0x06: "sub",
      0x07: "div",
      0x08: "mod",
      0x09: "inv",
      0x0a: "joinstr",
      0x0b: "joinpadstr",
      0x0c: "lt",
      0x0d: "lteq",
      0x0e: "nteq",
      0x0f: "eq",
      0x10: "gt",
      0x11: "gteq",
      0x12: "and",
      0x13: "or",
      0x14: "not",
      0x15: "containsstr",
      0x16: "contains0str",
      0x17: "splitstr",
      0x18: "lightstr",
      0x19: "ontospr",
      0x1a: "intospr",
      0x1b: "caststr",
      0x1c: "startobj",
      0x1d: "stopobj",
      0x1e: "wraplist",
      0x1f: "newproplist"
    };

    const multiByteCodes = {
      0x01: "pushint",
      0x02: "newarglist",
      0x03: "newlist",
      0x04: "pushcons",
      0x05: "pushsymb",
      0x09: "getglobal",
      0x0a: "getprop",
      0x0b: "getparam",
      0x0c: "getlocal",
      0x0f: "setglobal",
      0x10: "setprop",
      0x11: "setparam",
      0x12: "setlocal",
      0x13: "jmp",
      0x14: "endrepeat",
      0x15: "iftrue",
      0x16: "call_local",
      0x17: "call_external",
      0x18: "callobj_old?",
      0x19: "op_59xx",
      0x1b: "op_5bxx",
      0x1c: "get",
      0x1d: "set",
      0x1f: "getmovieprop",
      0x20: "setmovieprop",
      0x21: "getobjprop",
      0x22: "setobjprop",
      0x26: "getmovieinfo",
      0x27: "callobj",
      0x2e: "pushint"
    };

    let opcode = val < 0x40 ? oneByteCodes[val] : multiByteCodes[val % 0x40];
    return opcode || "unk_" + val.toString(16);
  }
}
