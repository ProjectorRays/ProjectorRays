# ProjectorRays Shockwave Decompiler

ProjectorRays is a decompiler for [Adobe Shockwave](https://en.wikipedia.org/wiki/Adobe_Shockwave) and [Adobe Director](https://en.wikipedia.org/wiki/Adobe_Director) (formerly known as Macromedia Shockwave and Macromedia Director, and not to be confused with [Shockwave Flash](https://en.wikipedia.org/wiki/Adobe_Flash)).

Director was released in 1987, and it quickly became the world's leading multimedia platform. Beginning in 1995, Director movies could be published as DCR files and played on the web with the Shockwave plugin. Over the years, the platform was used for countless CD-ROM and web games, before being fully discontinued in 2019.

Nowadays, Shockwave games are unplayable on the modern web, and their original source code is often lost or unavailable. ProjectorRays can take a published game, reconstruct its [Lingo](https://en.wikipedia.org/wiki/Lingo_(programming_language)) source code, and generate editable project files to aid preservation efforts.

If you have a DCR (published Shockwave movie) or DXR (protected Director movie), ProjectorRays can generate a DIR (editable Director movie).

If you have a CCT (published Shockwave cast) or CXT (protected Director cast), ProjectorRays can generate a CST (editable Director cast).

ProjectorRays is a work in progress. If you run into any issues, please report them on the [issue tracker](https://github.com/ProjectorRays/ProjectorRays/issues).

## How do I use it?

### Windows

Development builds are available on the [releases page](https://github.com/ProjectorRays/ProjectorRays/releases).

To use it, run `projectorrays.exe INPUT_FILE OUTPUT_FILE`. ProjectorRays will take the input file, unprotect/decompress it, restore its source code, and output it to the specified location. The outputted file can then be opened in Director.

### *nix

Install Boost 1.72.0 or later, nlohmann JSON, mpg123, and zlib. Run `make` to build.

To use it, run `./projectorrays INPUT_FILE OUTPUT_FILE`. ProjectorRays will take the input file, unprotect/decompress it, restore its source code, and output it to the specified location. The outputted file can then be opened in Director.

## Credits

ProjectorRays is written by [Deborah Servilla](https://github.com/djsrv), based on the [disassembler](https://github.com/Brian151/OpenShockwave/blob/50b3606809b3c8dad13ee41ae20bcbfa70eb3606/tools/lscrtoscript/js/projectorrays.js) by [Anthony Kleine](https://github.com/tomysshadow).

This could not exist without the reverse-engineering work of the [Earthquake Project](https://github.com/Earthquake-Project), the [Just Solve the File Format Problem wiki](http://fileformats.archiveteam.org/wiki/Lingo_bytecode), and the [ScummVM Director engine team](https://www.scummvm.org/credits/#:~:text=Director:).

The projector icon is by [DangerouslySlowCat](https://twitter.com/DangerSlowCat).

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
