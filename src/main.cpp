/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

#include "common/log.h"
#include "common/stream.h"
#include "common/util.h"
#include "director/chunk.h"
#include "director/dirfile.h"
#include "director/util.h"
#include "io/options.h"
#include "io/fileio.h"

using namespace Director;

bool processFile(fs::path input, IO::Options &options, bool outputIsDirectory) {
	std::vector<uint8_t> buf;
	if (!IO::readFile(input, buf)) {
		warning("Could not read %s!", input.c_str());
		return false;
	}

	Common::SeekableReadStream stream(buf.data(), buf.size());
	auto dir = std::make_unique<DirectorFile>();
	if (!dir->read(&stream))
		return false;

	fs::path decompileOutput;
	fs::path dumpOutput;
	if (options.hasOption("output") && !outputIsDirectory) {
		decompileOutput = options.stringValue("output");
	} else {
		std::string oldExtension = input.extension().string();
		std::string newExtension = (dir->isCast()) ? ".cst" : ".dir";
		std::string fileName = input.stem().string();
		std::string decompileFileName = (Common::compareIgnoreCase(oldExtension, newExtension) == 0)
											? fileName + "_decompiled" + newExtension
											: fileName + newExtension;
		std::string dumpDirName = (oldExtension.size() == 0)
										? fileName + "_dump"
										: fileName;
		if (options.hasOption("output") && outputIsDirectory) {
			decompileOutput = dumpOutput = fs::path(options.stringValue("output"));
			decompileOutput /= decompileFileName;
			dumpOutput /= dumpDirName;
		} else {
			decompileOutput = dumpOutput = input;
			decompileOutput.replace_filename(decompileFileName);
			dumpOutput.replace_filename(dumpDirName);
		}
	}
	if (options.hasDumpOptions()) {
		fs::create_directory(dumpOutput);
	}
	fs::path castsOutput = dumpOutput / std::string("casts");
	if (options.hasCastDumpOptions()) {
		fs::create_directory(castsOutput);
	}
	fs::path chunksOutput = dumpOutput / std::string("chunks");
	if (options.hasChunkDumpOptions()) {
		fs::create_directory(chunksOutput);
	}

	if (options.hasOption("dump-chunks")) {
		dir->dumpChunks(chunksOutput);
	}
	if (options.hasOption("dump-json")) {
		dir->dumpJSON(chunksOutput);
	}

	unsigned int version = humanVersion(dir->config->directorVersion);
	switch (options.cmd()) {
	case IO::kCmdDecompile:
		{
			dir->config->unprotect();
			dir->parseScripts();
			if (options.hasOption("dump-scripts")) {
				dir->dumpScripts(castsOutput);
			}
			dir->restoreScriptText();
			dir->writeToFile(decompileOutput);

			std::string fileType = (dir->isCast()) ? "cast" : "movie";
			Common::log(
				"Decompiled " + versionString(version, dir->fverVersionString) + " " + fileType
				+ " " + input.string() + " to " + decompileOutput.string()
			);
		}
		break;
	case IO::kCmdVersion:
		{
			IO::VersionStyle style = IO::kVersionStyleLong;
			if (options.hasOption("style")) {
				style = (IO::VersionStyle)options.enumValue("style");
			}
			switch (style) {
			case IO::kVersionStyleLong:
				Common::log(versionString(version, dir->fverVersionString));
				break;
			case IO::kVersionStyleShort:
				Common::log(versionNumber(version, dir->fverVersionString));
				break;
			case IO::kVersionStyleInteger:
				Common::log(std::to_string(version));
				break;
			case IO::kVersionStyleInternal:
				Common::log(std::to_string(dir->config->directorVersion));
				break;
			}
		}
		break;
	default:
		break;
	}

	return true;
}

int main(int argc, char *argv[]) {
	IO::Options options;
	options.parse(argc, argv);
	if (!options.valid()) {
		return EXIT_FAILURE;
	}
	if (options.hasOption("verbose")) {
		Common::g_verbose = true;
	}

	fs::path input = options.inputFile();
	if (fs::is_directory(input)) {
		if (options.hasOption("output")) {
			fs::path output = options.stringValue("output");
			if (fs::exists(output)) {
				if (!fs::is_directory(output)) {
					warning("Output must be a directory when input is a directory!");
					return EXIT_FAILURE;
				}
			} else {
				fs::create_directory(output);
			}
		}
		for (const fs::directory_entry &dirEntry : fs::directory_iterator(input)) {
			if (!dirEntry.is_regular_file())
				continue;

			fs::path path = dirEntry.path();
			std::string extension = path.extension().string();
			if (!(Common::compareIgnoreCase(extension, ".dcr") == 0
					|| Common::compareIgnoreCase(extension, ".dxr") == 0
					|| Common::compareIgnoreCase(extension, ".cct") == 0
					|| Common::compareIgnoreCase(extension, ".cxt") == 0))
				continue;

			if (!processFile(path, options, true))
				return EXIT_FAILURE;
		}
	} else {
		bool outputIsDirectory = false;
		if (options.hasOption("output")) {
			fs::path output = options.stringValue("output");
			if (fs::is_directory(output)) {
				outputIsDirectory = true;
			} else if (options.hasDumpOptions()) {
				warning("Output must be a directory when a --dump- option is used!");
				return EXIT_FAILURE;
			}
		}
		if (!processFile(input, options, outputIsDirectory))
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
