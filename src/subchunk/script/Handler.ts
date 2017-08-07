import {Script} from "../../chunk/Script";
import {SplitSubchunk} from "../SplitSubchunk";
import {Bytecode, AST, Stack} from "../../lingo";
import {DataStream} from "../../DataStream";
import * as lib from "../../lib";

/* Handler */

export class Handler implements SplitSubchunk {
  nameID: number;
  vectorPos: number;
  compiledLen: number;
  compiledOffset: number;
  argumentCount: number;
  argumentOffset: number;
  localsCount: number;
  localsOffset: number;
  unknown0Count: number;
  unknown0Offset: number;
  unknown1: number;
  unknown2: number;
  lineCount: number;
  lineOffset: number;
  stackHeight: number;
  argumentNameIDs: number[];
  localNameIDs: number[];

  bytecodeArray: Bytecode[];
  bytecodeByPos: Map<number, Bytecode>;
  argumentNames: string[];
  localNames: string[];
  name: string;
  ast: AST.AST;

  constructor(public script: Script) {}

  readRecord(dataStream: DataStream) {
    this.nameID = dataStream.readUint16();
    this.vectorPos = dataStream.readUint16();
    this.compiledLen = dataStream.readUint32();
    this.compiledOffset = dataStream.readUint32();
    this.argumentCount = dataStream.readUint16();
    this.argumentOffset = dataStream.readUint32();
    this.localsCount = dataStream.readUint16();
    this.localsOffset = dataStream.readUint32();
    this.unknown0Count = dataStream.readUint16();
    this.unknown0Offset = dataStream.readUint32();
    this.unknown1 = dataStream.readUint32();
    this.unknown2 = dataStream.readUint16();
    this.lineCount = dataStream.readUint16();
    this.lineOffset = dataStream.readUint32();
    // yet to implement
    this.stackHeight = dataStream.readUint32();
  }

  readData(dataStream: DataStream) {
    dataStream.seek(this.compiledOffset);
    this.bytecodeArray = [];
    this.bytecodeByPos = new Map();
    // seeks to the offset of the handlers. Currently only grabs the first handler in the script.
    // loop while there's still more code left
    while (dataStream.position < this.compiledOffset + this.compiledLen) {
      let pos = dataStream.position;
      let op = dataStream.readUint8();
      // instructions can be one, two or three bytes
      let obj = null, objLength = 0;
      if (op >= 0xc0) {
        obj = dataStream.readUint24();
        objLength = 3;
      } else if (op >= 0x80) {
        obj = dataStream.readUint16();
        objLength = 2;
      } else if (op >= 0x40) {
        obj = dataStream.readUint8();
        objLength = 1;
      }
      // read the first byte to convert to an opcode
      let bytecode = new Bytecode(op, obj, objLength, pos);
      this.bytecodeArray.push(bytecode);
      this.bytecodeByPos.set(pos, bytecode);
    }

    this.argumentNameIDs = this.readVarnamesTable(dataStream, this.argumentCount, this.argumentOffset);
    this.localNameIDs = this.readVarnamesTable(dataStream, this.localsCount, this.localsOffset);
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
    let nameList = this.script.context.scriptNames.names;
    this.name = nameList[this.nameID];
    this.argumentNames = this.argumentNameIDs.map(nameID => nameList[nameID]);
    this.localNames = this.localNameIDs.map(nameID => nameList[nameID]);
  }

  translate() {
    this.script.stack = new Stack();
    this.ast = new AST.AST(new AST.Handler(this.name, this.argumentNames));
    for (let i = 0, l = this.bytecodeArray.length; i < l; i++) {
      let bytecode = this.bytecodeArray[i];
      let pos = bytecode.pos;
      while (pos === this.ast.currentBlock.endPos) {
        let exitedBlock = this.ast.currentBlock;
        let blockParent = this.ast.currentBlock.parent;
        this.ast.exitBlock();
        if (blockParent.constructor === AST.IfStatement) {
          let ifStatement = blockParent as AST.IfStatement;
          if (ifStatement.type === AST.IfStatement.Type.if_else && exitedBlock === ifStatement.block1) {
            this.ast.enterBlock(ifStatement.block2);
          }
        }
      }
      this.translateBytecode(bytecode, i);
    }
  }

  translateBytecode(bytecode, index) {
    let translation = null;
    let stack = this.script.stack;
    let ast = this.ast;
    let nameList = this.script.context.scriptNames.names;

    const handleBinaryOperator = () => {
      const operators = {
        "mul": "*",
        "add": "+",
        "sub": "-",
        "div": "/",
        "mod": "mod",
        "joinstr": "&",
        "joinpadstr": "&&",
        "lt": "<",
        "lteq": "<=",
        "nteq": "<>",
        "eq": "=",
        "gt": ">",
        "gteq": ">=",
        "and": "and",
        "or": "or",
        "containsstr": "contains",
        "contains0str": "starts"
      };
      let y = stack.pop();
      let x = stack.pop();
      translation = new AST.BinaryOperator(operators[bytecode.opcode], x, y);
      stack.push(translation);
    };

    const bytecodeHandlers = {
      "ret": () => {
        translation = new AST.ExitStatement();
        ast.addStatement(translation);
      },
      "pushint0": () => {
        translation = new AST.IntLiteral(0);
        stack.push(translation);
      },
      "mul": handleBinaryOperator,
      "add": handleBinaryOperator,
      "sub": handleBinaryOperator,
      "div": handleBinaryOperator,
      "mod": handleBinaryOperator,
      "inv": () => {
        let x = stack.pop();
        translation = new AST.InverseOperator(x);
        stack.push(translation);
      },
      "joinstr": handleBinaryOperator,
      "joinpadstr": handleBinaryOperator,
      "lt": handleBinaryOperator,
      "lteq": handleBinaryOperator,
      "nteq": handleBinaryOperator,
      "eq": handleBinaryOperator,
      "gt": handleBinaryOperator,
      "gteq": handleBinaryOperator,
      "and": handleBinaryOperator,
      "or": handleBinaryOperator,
      "not": () => {
        let x = stack.pop();
        translation = new AST.NotOperator(x);
        stack.push(translation);
      },
      "containsstr": handleBinaryOperator,
      "contains0str": handleBinaryOperator,
      "splitstr": () => {
        let string = stack.pop();
        let lastLine = stack.pop();
        let firstLine = stack.pop();
        let lastItem = stack.pop();
        let firstItem = stack.pop();
        let lastWord = stack.pop();
        let firstWord = stack.pop();
        let lastChar = stack.pop();
        let firstChar = stack.pop();
        if (firstChar.getValue() !== 0) {
          translation = new AST.StringSplitExpression("char", firstChar, lastChar, string);
        } else if (firstWord.getValue() !== 0) {
          translation = new AST.StringSplitExpression("word", firstWord, lastWord, string);
        } else if (firstItem.getValue() !== 0) {
          translation = new AST.StringSplitExpression("item", firstItem, lastItem, string);
        } else if (firstLine.getValue() !== 0) {
          translation = new AST.StringSplitExpression("line", firstLine, lastLine, string);
        }
        stack.push(translation);
      },
      "lightstr": () => {
        let field = stack.pop();
        let lastLine = stack.pop();
        let firstLine = stack.pop();
        let lastItem = stack.pop();
        let firstItem = stack.pop();
        let lastWord = stack.pop();
        let firstWord = stack.pop();
        let lastChar = stack.pop();
        let firstChar = stack.pop();
        if (firstChar.getValue() !== 0) {
          translation = new AST.StringHilightStatement("char", firstChar, lastChar, field);
        } else if (firstWord.getValue() !== 0) {
          translation = new AST.StringHilightStatement("word", firstWord, lastWord, field);
        } else if (firstItem.getValue() !== 0) {
          translation = new AST.StringHilightStatement("item", firstItem, lastItem, field);
        } else if (firstLine.getValue() !== 0) {
          translation = new AST.StringHilightStatement("line", firstItem, lastItem, field);
        }
        ast.addStatement(translation);
      },
      "ontospr": () => {
        let firstSprite = stack.pop();
        let secondSprite = stack.pop();
        translation = new AST.SpriteIntersectsExpression(firstSprite, secondSprite);
        stack.push(translation);
      },
      "intospr": () => {
        let firstSprite = stack.pop();
        let secondSprite = stack.pop();
        translation = new AST.SpriteWithinExpression(firstSprite, secondSprite);
        stack.push(translation);
      },
      "caststr": () => {
        let fieldID = stack.pop();
        translation = new AST.FieldReference(fieldID);
        stack.push(translation);
      },
      "startobj": () => {
        stack.pop();
        // TODO
      },
      "stopobj": () => {
        // TODO
      },
      "wraplist": () => {
        let list = stack.pop();
        stack.push(list);
      },
      "newproplist": () => {
        let list = stack.pop().getValue();
        translation = new AST.PropListLiteral(list);
        stack.push(translation);
      },
      "pushint": () => {
        translation = new AST.IntLiteral(bytecode.obj);
        stack.push(translation);
      },
      "newarglist": () => {
        let args = stack.splice(stack.length - bytecode.obj, bytecode.obj);
        translation = new AST.ArgListLiteral(args);
        stack.push(translation);
      },
      "newlist": () => {
        let items = stack.splice(stack.length - bytecode.obj, bytecode.obj);
        translation = new AST.ListLiteral(items);
        stack.push(translation);
      },
      "pushcons": () => {
        let literal = this.script.literals[bytecode.obj]
        let type = lib.LiteralTypes[literal.type];
        if (type === "string") {
          translation = new AST.StringLiteral(literal.value);
        } else if (type === "int") {
          translation = new AST.IntLiteral(literal.value);
        } else if (type === "float") {
          translation = new AST.FloatLiteral(literal.value);
        }
        stack.push(translation);
      },
      "pushsymb": () => {
        translation = new AST.SymbolLiteral(nameList[bytecode.obj]);
        stack.push(translation);
      },
      "getglobal": () => {
        translation = new AST.GlobalVarReference(nameList[bytecode.obj]);
        stack.push(translation);
      },
      "getprop": () => {
        translation = new AST.PropertyReference(nameList[bytecode.obj]);
        stack.push(translation);
      },
      "getparam": () => {
        translation = new AST.ParamReference(this.argumentNames[bytecode.obj]);
        stack.push(translation);
      },
      "getlocal": () => {
        translation = new AST.LocalVarReference(this.localNames[bytecode.obj]);
        stack.push(translation);
      },
      "setglobal": () => {
        let value = stack.pop();
        translation = new AST.AssignmentStatement(new AST.GlobalVarReference(nameList[bytecode.obj]), value);
        ast.addStatement(translation);
      },
      "setprop": () => {
        let value = stack.pop();
        translation = new AST.AssignmentStatement(new AST.PropertyReference(nameList[bytecode.obj]), value);
        ast.addStatement(translation);
      },
      "setparam": () => {
        let value = stack.pop();
        translation = new AST.AssignmentStatement(new AST.ParamReference(this.argumentNames[bytecode.obj]), value);
        ast.addStatement(translation);
      },
      "setlocal": () => {
        let value = stack.pop();
        translation = new AST.AssignmentStatement(new AST.LocalVarReference(this.localNames[bytecode.obj]), value);
        ast.addStatement(translation);
      },
      "jmp": () => {
        let targetPos = bytecode.pos + bytecode.obj;
        let nextBytecode = this.bytecodeArray[index + 1];
        let targetBytecode = this.bytecodeByPos.get(targetPos);
        let targetPrevBytecode = this.bytecodeArray[index - 1];
        let blockParent = ast.currentBlock.parent;

        if (blockParent.constructor === AST.IfStatement) {
          let ifStatement = blockParent as AST.IfStatement
          if (nextBytecode.pos === ast.currentBlock.endPos && targetPrevBytecode.opcode === "endrepeat") {
            translation = new AST.ExitRepeatStatement();
            ast.addStatement(translation);
          } else if (targetBytecode.opcode === "endrepeat") {
            translation = new AST.NextRepeatStatement();
            ast.addStatement(translation);
          } else if (nextBytecode.pos === ast.currentBlock.endPos) {
            ifStatement.setType(AST.IfStatement.Type.if_else);
            ifStatement.block2.endPos = targetPos;
          }
        }
      },
      "endrepeat": () => {
        let targetPos = bytecode.pos - bytecode.obj;
        let targetBytecode = this.bytecodeByPos.get(targetPos);
        let i = this.bytecodeArray.indexOf(targetBytecode);
        while (this.bytecodeArray[i].opcode !== "iftrue") i++;
        targetBytecode = this.bytecodeArray[i];
        let ifStatement = targetBytecode.translation as AST.IfStatement;
        ifStatement.setType(AST.IfStatement.Type.repeat_while);
      },
      "iftrue": () => {
        let endPos = bytecode.pos + bytecode.obj
        let condition = stack.pop();
        translation = new AST.IfStatement(AST.IfStatement.Type.if, condition);
        translation.block1.endPos = endPos;
        ast.addStatement(translation);
        ast.enterBlock(translation.block1)
      },
      "call_local": () => {
        let argList = stack.pop();
        translation = new AST.LocalCallStatement(this.script.handlers[bytecode.obj].name, argList);
        if (argList.constructor === AST.ListLiteral) {
          stack.push(translation);
        } else {
          ast.addStatement(translation);
        }
      },
      "call_external": () => {
        let argList = stack.pop();
        translation = new AST.ExternalCallStatement(nameList[bytecode.obj], argList);
        if (argList.constructor === AST.ListLiteral) {
          stack.push(translation);
        } else {
          ast.addStatement(translation);
        }
      },
      "callobj_old?": () => {
        // Possibly used by old Director versions?
        let object = stack.pop();
        let argList = stack.pop();
        // TODO
      },
      "op_59xx": () => {
        stack.pop();
        // TODO
      },
      "op_5bxx": () => {
        stack.pop();
        // TODO
      },
      "get": () => {
        switch (bytecode.obj) {
          case 0x00:
            (() => {
              let id = stack.pop().getValue();
              if (id <= 0x05) {
                translation = new AST.MoviePropertyReference(lib.MoviePropertyNames00[id]);
              } else if (id <= 0x0b) {
                translation = new AST.TimeExpression(lib.TimeNames[id - 0x05]);
              } else {
                let string = stack.pop();
                translation = new AST.LastStringChunkExpression(lib.ChunkTypeNames[id - 0x0b], string);
              }
            })();
            break;
          case 0x01:
            (() => {
              let statID = stack.pop().getValue();
              let string = stack.pop();
              translation = new AST.StringChunkCountExpression(lib.ChunkTypeNames[statID], string);
            })();
            break;
          case 0x02:
            (() => {
              let propertyID = stack.pop().getValue();
              let menuID = stack.pop();
              translation = new AST.MenuPropertyReference(menuID, lib.MenuPropertyNames[propertyID]);
            })();
            break;
          case 0x03:
            (() => {
              let propertyID = stack.pop().getValue();
              let menuID = stack.pop();
              let itemID = stack.pop();
              translation = new AST.MenuItemPropertyReference(menuID, itemID, lib.MenuItemPropertyNames[propertyID]);
            })();
            break;
          case 0x04:
            (() => {
              let propertyID = stack.pop().getValue();
              let soundID = stack.pop();
              translation = new AST.SoundPropertyReference(soundID, lib.SoundPropertyNames[propertyID]);
            })();
            break;
          case 0x06:
            (() => {
              let propertyID = stack.pop().getValue();
              let spriteID = stack.pop();
              translation = new AST.SpritePropertyReference(spriteID, lib.SpritePropertyNames[propertyID]);
            })();
            break;
          case 0x07:
            (() => {
              let settingID = stack.pop().getValue();
              translation = new AST.MoviePropertyReference(lib.MoviePropertyNames07[settingID]);
            })();
            break;
          case 0x08:
            (() => {
              let statID = stack.pop().getValue();
              if (statID === 0x01) {
                translation = new AST.MoviePropertyReference("perFrameHook");
              } else {
                translation = new AST.ObjCountExpression(lib.CountableObjectNames[statID]);
              }
            })();
            break;
          case 0x09:
            (() => {
              let propertyID = stack.pop().getValue();
              let castID = stack.pop();
              translation = new AST.SpritePropertyReference(castID, lib.CastPropertyNames09[propertyID]);
            })();
            break;
          case 0x0c:
            (() => {
              let propertyID = stack.pop().getValue();
              let fieldID = stack.pop();
              translation = new AST.FieldPropertyReference(fieldID, lib.FieldPropertyNames[propertyID]);
            })();
            break;
          case 0x0d:
            (() => {
              let propertyID = stack.pop().getValue();
              let castID = stack.pop();
              translation = new AST.SpritePropertyReference(castID, lib.CastPropertyNames0d[propertyID]);
            })();
        }
        stack.push(translation);
      },
      "set": () => {
        switch (bytecode.obj) {
          case 0x00:
            (() => {
              let id = stack.pop().getValue();
              let value = stack.pop();
              if (id <= 0x05) {
                translation = new AST.AssignmentStatement(
                  new AST.MoviePropertyReference(lib.MoviePropertyNames00[id]),
                  value
                );
              }
            })();
            break;
          case 0x03:
            (() => {
              let propertyID = stack.pop().getValue();
              let value = stack.pop();
              let menuID = stack.pop();
              let itemID = stack.pop();
              translation = new AST.AssignmentStatement(
                new AST.MenuItemPropertyReference(menuID, itemID, lib.MenuItemPropertyNames[propertyID]),
                value
              );
            })();
            break;
          case 0x04:
            (() => {
              let propertyID = stack.pop().getValue();
              let value = stack.pop();
              let soundID = stack.pop();
              translation = new AST.AssignmentStatement(
                new AST.SoundPropertyReference(soundID, lib.SoundPropertyNames[propertyID]),
                value
              );
            })();
            break;
          case 0x06:
            (() => {
              let propertyID = stack.pop().getValue();
              let value = stack.pop();
              let spriteID = stack.pop();
              translation = new AST.AssignmentStatement(
                new AST.SpritePropertyReference(spriteID, lib.SpritePropertyNames[propertyID]),
                value
              );
            })();
            break;
          case 0x07:
            (() => {
              let settingID = stack.pop().getValue();
              let value = stack.pop();
              translation = new AST.AssignmentStatement(
                new AST.MoviePropertyReference(lib.MoviePropertyNames07[settingID]),
                value
              );
            })();
            break;
          case 0x09:
            (() => {
              let propertyID = stack.pop().getValue();
              let value = stack.pop();
              let castID = stack.pop();
              translation = new AST.AssignmentStatement(
                new AST.SpritePropertyReference(castID, lib.CastPropertyNames09[propertyID]),
                value
              );
            })();
            break;
          case 0x0c:
            (() => {
              let propertyID = stack.pop().getValue();
              let value = stack.pop();
              let fieldID = stack.pop();
              translation = new AST.AssignmentStatement(
                new AST.FieldPropertyReference(fieldID, lib.FieldPropertyNames[propertyID]),
                value
              );
            })();
            break;
          case 0x0d:
            (() => {
              let propertyID = stack.pop().getValue();
              let value = stack.pop();
              let castID = stack.pop();
              translation = new AST.AssignmentStatement(
                new AST.SpritePropertyReference(castID, lib.CastPropertyNames0d[propertyID]),
                value
              );
            })();
        }
        ast.addStatement(translation);
      },
      "getmovieprop": () => {
        translation = new AST.MoviePropertyReference(nameList[bytecode.obj]);
        stack.push(translation);
      },
      "setmovieprop": () => {
        let value = stack.pop();
        translation = new AST.AssignmentStatement(new AST.MoviePropertyReference(nameList[bytecode.obj]), value);
        ast.addStatement(translation);
      },
      "getobjprop": () => {
        let object = stack.pop();
        translation = new AST.ObjPropertyReference(object, nameList[bytecode.obj]);
        stack.push(translation);
      },
      "setobjprop": () => {
        let value  = stack.pop();
        let object = stack.pop();
        translation = new AST.AssignmentStatement(new AST.ObjPropertyReference(object, nameList[bytecode.obj]), value);
        ast.addStatement(translation);
      },
      "getmovieinfo": () => {
        stack.pop();
        translation = new AST.MoviePropertyReference(nameList[bytecode.obj]);
        stack.push(translation);
      },
      "callobj": () => {
        let argList = stack.pop();
        let obj = argList.shift();
        translation = new AST.ObjCallStatement(obj, nameList[bytecode.obj], argList);
        if (argList.constructor === AST.ListLiteral) {
          stack.push(translation);
        } else {
          ast.addStatement(translation);
        }
      }
    };

    if (typeof bytecodeHandlers[bytecode.opcode] === "function") {
      bytecodeHandlers[bytecode.opcode]();
    } else {
      translation = new AST.ERROR();
      ast.addStatement(new AST.Comment(bytecode.opcode.toUpperCase() + (bytecode.obj != null ? " " + bytecode.obj : "")));
      this.script.stack = new Stack(); // Clear stack so later bytecode won't be too screwed up
    }
    bytecode.translation = translation;
  }
}
