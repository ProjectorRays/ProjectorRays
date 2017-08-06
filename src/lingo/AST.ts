/* AST */

export class AST {
  currentBlock: Block;

  constructor(public root: Handler) {
    this.currentBlock = this.root.block;
  }

  toString() {
    return this.root.toString();
  }

  addStatement(statement: Node) {
    this.currentBlock.addChild(statement);
  }

  enterBlock(block: Block) {
    this.currentBlock = block;
  }

  exitBlock() {
    this.currentBlock = this.currentBlock.parent.parent as Block;
  }
}

/* Node */

export class Node {
  parent: Node;

  toString() {
    return "";
  }

  toPseudocode() {
    return this.toString();
  }

  getValue() {
    return true; // placeholder...
  }
}

/* TODO */

export class TODO extends Node {
  toString() {
    return "TODO";
  }
}

/* ERROR */

export class ERROR extends Node {
  toString() {
    return "ERROR";
  }
}

/* Comment */

export class Comment extends Node {
  constructor(public text: string) {
    super();
  }

  toString() {
    return "-- " + this.text;
  }
}

/* Literal */

class Literal extends Node {
  constructor(public value: any) {
    super();
  }

  toString() {
    return this.value.toString();
  }

  getValue() {
    return this.value;
  }
}

/* StringLiteral */

export class StringLiteral extends Literal {
  toString() {
    return JSON.stringify(this.value);
  }
}

/* IntLiteral */

export class IntLiteral extends Literal {}

/* FloatLiteral */

export class FloatLiteral extends Literal {}

/* ListLiteral */

export class ListLiteral extends Literal {
  toString() {
    return "[" + this.value.join(", ") + "]";
  }

  toStringNoBrackets() {
    return this.value.join(", ");
  }

  pop() {
    return this.value.pop();
  }

  shift() {
    return this.value.shift() || new ERROR();
  }
}

/* ArgListLiteral */

export class ArgListLiteral extends ListLiteral {}

/* PropListLiteral */

export class PropListLiteral extends Literal {
  toString() {
    var result = "[";
    for (let i = 0, l = this.value.length; i < l; i += 2) {
      result += this.value[i] + ":" + this.value[i + 1];
      if (i < l - 2) result += ", ";
    }
    result += "]";
    return result;
  }
}

/* SymbolLiteral */

export class SymbolLiteral extends Literal {
  toString() {
    return "#" + this.value;
  }
}

/* Block */

export class Block extends Node {
  endPos: number;

  constructor(public children: Node[] = []) {
    super();
  }

  toString() {
    const indent = "\n  ";
    if (this.children.length > 0){
      return indent + this.children.map(child => child.toString().split("\n").join(indent)).join(indent);
    }
    return "";
  }

  addChild(child) {
    this.children.push(child);
    if (child) child.parent = this;
  }
}

/* Handler */

export class Handler extends Node {
  constructor(public name: string, public args: string[], public block: Block = null) {
    super();
    if (!this.block) {
      this.block = new Block();
    }
    this.block.parent = this;
  }

  toString() {
    return "on " + this.name + "(" + this.args.join(", ") + ")" + this.block + "\nend";
  }
}

/* ExitStatement */

export class ExitStatement extends Node {
  toString() {
    return "exit";
  }
}

/* InverseOperator */

export class InverseOperator extends Node {
  constructor(public operand: Node) {
    super();
    this.operand.parent = this;
  }

  toString() {
    return "-" + this.operand;
  }
}

/* NotOperator */

export class NotOperator extends Node {
  constructor(public operand: Node) {
    super();
    this.operand.parent = this;
  }

  toString() {
    return "not " + this.operand;
  }
}

/* BinaryOperator */

export class BinaryOperator extends Node {
  constructor(public name: string, public left: Node, public right: Node) {
    super();
    this.name = name;
    this.left.parent = this;
    this.right.parent = this;
  }

  toString() {
    return this.left + " " + this.name + " " + this.right;
  }
}

/* StringSplitExpression */

export class StringSplitExpression extends Node {
  constructor(public type: string, public first: Node, public last: Node, public string: Node) {
    super();
    this.type = type;
    this.first.parent = this;
    this.last.parent = this;
    this.string.parent = this;
  }

  toString() {
    var result = this.string + "." + this.type + "[" + this.first;
    if (this.last.getValue()) {
      result += ".." + this.last;
    }
    result += "]";
    return result;
  }
}

/* StringHilightStatement */

export class StringHilightStatement extends StringSplitExpression {
  constructor(type, first, last, string) {
    super(type, first, last, string);
  }

  toString() {
    let result = super.toString();
    result += ".hilite()";
    return result;
  }
}

/* SpriteIntersectsExpression */

export class SpriteIntersectsExpression extends Node {
  constructor(public firstSprite: Node, public secondSprite: Node) {
    super();
    this.firstSprite.parent = this;
    this.secondSprite.parent = this;
  }

  toString() {
    return "sprite(" + this.firstSprite + ").intersects(" + this.secondSprite + ")";
  }
}

/* SpriteWithinExpression */

export class SpriteWithinExpression extends Node {
  constructor(public firstSprite: Node, public secondSprite: Node) {
    super();
    this.firstSprite.parent = this;;
    this.secondSprite.parent = this;
  }

  toString() {
    return "sprite(" + this.firstSprite + ").within(" + this.secondSprite + ")";
  }
}

/* FieldReference */

export class FieldReference extends Node {
  constructor(public fieldID: Node) {
    super();
    this.fieldID.parent = this;
  }

  toString() {
    return "field(" + this.fieldID + ")";
  }
}

/* VarReference */

export class VarReference extends Node {
  constructor(public varName: string) {
    super();
  }

  toString() {
    return this.varName;
  }
}

/* GlobalVarReference */

export class GlobalVarReference extends VarReference {}

/* PropertyReference */

export class PropertyReference extends VarReference {}

/* LocalVarReference */

export class LocalVarReference extends VarReference {}

/* ParamReference */

export class ParamReference extends VarReference {}

/* VarAssignment */

export class AssignmentStatement extends Node {
  constructor(public variable: Node, public value: Node) {
    super();
    this.variable.parent = this;
    this.value.parent = this;
  }

  toString() {
    return this.variable + " = " + this.value;
  }
}

/* IfStatement */

export class IfStatement extends Node {
  constructor(public type: string = "if", public condition: Node, public block1: Block = null, public block2: Block = null) {
    super();
    this.condition.parent = this;
    if (!this.block1) {
      this.block1 = new Block();
    }
    this.block1.parent = this;
    if (!this.block2 && this.type === "if_else") {
      block2 = new Block();
    }
    if (this.block2) {
      this.block2.parent = this;
    }
  }

  toString() {
    if (this.type === "if") {
      return "if " + this.condition + " then" + this.block1 + "\nend if";
    } else if (this.type === "if_else") {
      return "if " + this.condition + " then" + this.block1 + "\nelse" + this.block2 + "\nend if";
    } else if (this.type === "repeat_while") {
      return "repeat while " + this.condition + this.block1 + "\nend repeat";
    }
  }

  toPseudocode() {
    if (this.type === "if") {
      return "if " + this.condition + " then";
    } else if (this.type === "if_else") {
      return "if " + this.condition + " then";
    } else if (this.type === "repeat_while") {
      return "repeat while " + this.condition;
    }
  }

  setType(type) {
    this.type = type;
    if (this.type === "if_else") {
      this.block2 = new Block();
      this.block2.parent = this;
    } else {
      this.block2 = null;
    }
  }
}

/* CallStatement */

export class CallStatement extends Node {
  constructor(public name: string, public argList: ListLiteral) {
    super();
    this.argList.parent = this;
  }

  toString() {
    return this.name + "(" + this.argList.toStringNoBrackets() + ")";
  }
}

/* LocalCallStatement */

export class LocalCallStatement extends CallStatement {}

/* ExternalCallStatement */

export class ExternalCallStatement extends CallStatement {}

/* ObjCallStatement */

export class ObjCallStatement extends CallStatement {
  constructor(public obj: Node, name: string, argList: ListLiteral) {
    super(name, argList);
    this.obj.parent = this;
  }

  toString() {
    return this.obj + "." + super.toString();
  }
}

/* MoviePropertyReference */

export class MoviePropertyReference extends Node {
  constructor(public propertyName: String) {
    super();
  }

  toString() {
    return "the " + this.propertyName;
  }
}

/* TimeExpression */

export class TimeExpression extends Node {
  constructor(public option: string) {
    super();
  }

  toString() {
    return "the " + this.option;
  }
}

/* LastStringChunkExpression */

export class LastStringChunkExpression extends Node {
  constructor(public chunkType: string, public string: Node) {
    super();
    this.string.parent = this;
  }

  toString() {
    return "the last " + this.chunkType + " in " + this.string;
  }
}

/* StringChunkCountExpression */

export class StringChunkCountExpression extends Node {
  constructor(public chunkType: string, public string: Node) {
    super();
    this.string.parent = this;
  }

  toString() {
    return "the number of " + this.chunkType + " in " + this.string;
  }
}

/* MenuPropertyReference */

export class MenuPropertyReference extends Node {
  constructor(public menuID: Node, public propertyName: string) {
    super();
    this.menuID.parent = this;
  }

  toString() {
    return "menu(" + this.menuID + ")." + this.propertyName;
  }
}

/* MenuItemPropertyReference */

export class MenuItemPropertyReference extends Node {
  constructor(public menuID: Node, public itemID: Node, public propertyName: string) {
    super();
    this.menuID.parent = this;
    this.itemID.parent = this;
  }

  toString() {
    return "menu(" + this.menuID + ").item(" + this.itemID + ")." + this.propertyName;
  }
}

/* SoundPropertyReference */

export class SoundPropertyReference extends Node {
  constructor(public soundID: Node, public propertyName: string) {
    super();
    this.soundID.parent = this;
  }

  toString() {
    return "sound(" + this.soundID + ")." + this.propertyName;
  }
}

/* SpritePropertyReference */

export class SpritePropertyReference extends Node {
  constructor(public spriteID: Node, public propertyName: string) {
    super();
    this.spriteID.parent = this;
  }

  toString() {
    return "sprite(" + this.spriteID + ")." + this.propertyName;
  }
}

/* ObjCountExpression */

export class ObjCountExpression extends Node {
  constructor(public obj: string) {
    super();
  }

  toString() {
    return "the number of " + this.obj + "s";
  }
}

/* CastPropertyReference */

export class CastPropertyReference extends Node {
  constructor(public castID: Node, public propertyName: string) {
    super();
    this.castID.parent = this;
  }

  toString() {
    return "sprite(" + this.castID + ")." + this.propertyName;
  }
}

/* FieldPropertyReference */

export class FieldPropertyReference extends Node {
  constructor(public fieldID: Node, public propertyName: string) {
    super();
    this.fieldID.parent = this;
  }

  toString() {
    return "field(" + this.fieldID + ")." + this.propertyName;
  }
}

/* ObjPropertyReference */

export class ObjPropertyReference extends Node {
  constructor(public obj: Node, public propertyName: string) {
    super();
    this.obj.parent = this;
  }

  toString() {
    return this.obj + "." + this.propertyName;
  }
}

/* ExitRepeatStatement */

export class ExitRepeatStatement extends Node {
  toString() {
    return "exit repeat";
  }
}

/* NextRepeatStatement */

export class NextRepeatStatement extends Node {
  toString() {
    return "next repeat";
  }
}
