# ProjectorRays Shockwave Decompiler

ProjectorRays is a decompiler for [Adobe Shockwave](https://en.wikipedia.org/wiki/Adobe_Shockwave) and [Adobe Director](https://en.wikipedia.org/wiki/Adobe_Director) (formerly known as Macromedia Shockwave and Macromedia Director, and not to be confused with [Shockwave Flash](https://en.wikipedia.org/wiki/Adobe_Flash)).

Director was released in 1987 and for decades was a leading multimedia platform. In 1995, Director made its debut on the World Wide Web, allowing Director movies to be "afterburned" into DCR files which could be played by the Shockwave browser plugin. Shockwave was discontinued in 2019, leaving a wealth of old web games unplayable.

Shockwave games were programmed in [Lingo](https://en.wikipedia.org/wiki/Lingo_(programming_language)), but nowadays, their original source code is often lost or unavailable. Fortunately, ProjectorRays can reconstruct this lost source code from compiled Lingo bytecode.

ProjectorRays currently supports these file types:

| File extension | Description |
| - | - |
| DCR | Shockwave movie |
| DIR | Director movie |
| DXR | Protected Director movie |
| CCT | Shockwave cast |
| CST | Director cast |
| CXT | Protected Director cast |

## How do I use it?

### Windows

Development builds are available on the [releases page](https://github.com/ProjectorRays/ProjectorRays/releases).

To use it, drop a movie or cast file on `projectorrays.exe`. ProjectorRays will output `.ls` files with decompiled source code in the same folder as the movie.

### *nix

Install Boost 1.72.0 or later and zlib. Run `make` to build.

To use it, run `./projectorrays FILE`. ProjectorRays will output `.ls` files with decompiled source code in the current working directory.

## Credits

ProjectorRays is written by [Dylan Servilla](https://github.com/djsrv), based on the [disassembler](https://github.com/Brian151/OpenShockwave/blob/50b3606809b3c8dad13ee41ae20bcbfa70eb3606/tools/lscrtoscript/js/projectorrays.js) by [Anthony Kleine](https://github.com/tomysshadow).

This could not exist without the reverse-engineering work of the [Earthquake Project](https://github.com/Earthquake-Project), the [Just Solve the File Format Problem wiki](http://fileformats.archiveteam.org/wiki/Lingo_bytecode), and the [ScummVM Director engine team](https://www.scummvm.org/credits/#:~:text=Director:).

The projector icon is by [DangerouslySlowCat](https://twitter.com/DangerSlowCat).

## License

ProjectorRays is dual-licensed under the Apache License 2.0 and the MIT License.
