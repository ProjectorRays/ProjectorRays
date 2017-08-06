import {BaseError} from "make-error";

/* InvalidDirectorFileError */

export class InvalidDirectorFileError extends BaseError {
  constructor(message: string) {
    super(message);
  }
}

/* PathTooNewError */

export class PathTooNewError extends BaseError {
  constructor(message: string) {
    super(message);
  }
}
