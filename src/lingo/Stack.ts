import * as AST from "./AST";

/* Stack */

export class Stack extends Array {
    pop() {
        return this.length > 0 ? Array.prototype.pop.apply(this) : new AST.ERROR();
    }
}
