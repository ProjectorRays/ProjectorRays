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

## How do I use it?

The only dependencies besides the standard library are Boost and zlib. Once you have these, run `make` and then `./projectorrays MOVIE_FILE`

ProjectorRays will output `.lingo` files with guessed source code in the current working directory.

Once the project is more stable, a pre-built GUI application will be available.

## License

ProjectorRays is dual-licensed under the Apache License 2.0 and the MIT License.
