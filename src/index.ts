import * as minimist from "minimist";
import * as path from "path";
import * as fs from "fs-extra";
import * as json from "format-json";
import {Movie} from "./Movie";
import {ScriptContext} from "./chunk";

interface Args extends minimist.ParsedArgs {}

let args = minimist<Args>(process.argv.slice(2));
let inputPath = path.normalize(args._[0]);
let outputPath = path.normalize(path.parse(inputPath).name);

let buffer = fs.readFileSync(inputPath);
let movie = new Movie();
movie.read(buffer);

fs.emptyDirSync(outputPath);

let chunksPath = path.resolve(outputPath, "chunks");
fs.emptyDirSync(chunksPath);
for (let chunkID in movie.chunkMap) {
  let chunk = movie.chunkMap[chunkID];
  fs.writeFileSync(path.resolve(chunksPath, chunk.name + "_" + chunkID + ".bin"), movie._chunkBuffers.get(chunk));
}

let ctxArray = movie.chunkArrays.get("LctX") as ScriptContext[];
let lingoPath = path.resolve(outputPath, "lingo");
fs.emptyDirSync(lingoPath);
for (let context of ctxArray) {
  let contextID = movie.chunkMap.indexOf(context);
  let contextPath = path.resolve(lingoPath, "LctX_" + contextID);
  fs.emptyDirSync(contextPath);
  for (let script of context.scripts) {
    let scriptID = movie.chunkMap.indexOf(script);
    fs.writeFileSync(path.resolve(contextPath, "Lscr_" + scriptID + ".ls"), script.toString());
  }
}
