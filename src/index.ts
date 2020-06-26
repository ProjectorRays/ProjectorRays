import * as minimist from "minimist";
import * as path from "path";
import * as fs from "fs-extra";
import * as json from "format-json";
import {Movie} from "./Movie";
import * as Chunk from "./chunk";

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
    fs.writeFileSync(path.resolve(chunksPath, chunk.fourCC + "_" + chunkID + ".bin"), movie._chunkBuffers.get(chunk));
}

let lingoPath = path.resolve(outputPath, "lingo");
fs.emptyDirSync(lingoPath);
let ctxArray = movie.chunkArrays.get("LctX") as Chunk.ScriptContext[];
for (let context of ctxArray) {
    let contextID = movie.chunkMap.indexOf(context);
    let contextPath = path.resolve(lingoPath, "LctX_" + contextID);
    fs.emptyDirSync(contextPath);
    for (let script of context.scripts) {
        let scriptSrc = script.toString();
        fs.writeFileSync(path.resolve(contextPath, script.scriptNumber + ".ls"), scriptSrc);
    }
}

let castsPath = path.resolve(outputPath, "casts");
fs.emptyDirSync(castsPath);
for (let castID in movie.castMap) {
    let cast = movie.castMap[castID];
    let castPath = path.resolve(castsPath, cast.name);
    fs.emptyDirSync(castPath);
    let ctxArray = cast.chunkArrays.get("LctX") as Chunk.ScriptContext[];
    if (!ctxArray) continue;
    for (let context of ctxArray) {
        let contextID = movie.chunkMap.indexOf(context);
        let contextPath = path.resolve(castPath, "LctX_" + contextID);
        fs.emptyDirSync(contextPath);
        for (let i = 0, l = context.scripts.length; i < l; i++) {
            let script = context.scripts[i];
            let castMember = cast.scripts[script.scriptNumber];
            if (!castMember) continue;
            let scriptName = castMember.name || "unnamed";
            let scriptSrc = script.toString();
            fs.writeFileSync(path.resolve(contextPath, scriptName + "_" + script.scriptNumber + ".ls"), scriptSrc);
        }
    }
}
