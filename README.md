# ProjectorRays

## What is this?

ProjectorRays is an experimental Lingo decompiler for Shockwave movies, written in TypeScript. Right now it can take a `.dir` file and spit out the individual chunks and some guessed Lingo.

ProjectorRays is used as a testing ground for AfterShock, which will be a full-fledged Shockwave decompiler and editor written in a less terrible language.

## How do I use it?

Probably don't. There's a good chance something won't work quite right, and our focus is going to switch to AfterShock very soon, so there's no guarantee it will be fixed.

If you can help fix bugs, you can compile it by running
```
npm install
gulp
```
and use it with `npm start <path_to_dir_file>`
