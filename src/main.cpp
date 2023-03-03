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

#include "common/options.h"
#include "common/fileio.h"
#include "common/log.h"
#include "common/stream.h"
#include "common/util.h"
#include "director/chunk.h"
#include "director/dirfile.h"
#include "director/util.h"

using namespace Director;

bool processFile(fs::path input, Common::Options &options, bool outputIsDirectory) {
	std::vector<uint8_t> buf;
	if (!Common::readFile(input, buf)) {
		Common::warning(boost::format("Could not read %s!") % input);
		return false;
	}

	Common::ReadStream stream(buf.data(), buf.size());
	auto dir = std::make_unique<DirectorFile>();
	if (!dir->read(&stream))
		return false;

	if (options.hasOption("dump-chunks")) {
		dir->dumpChunks();
	}
	if (options.hasOption("dump-json")) {
		dir->dumpJSON();
	}

	unsigned int version = humanVersion(dir->config->directorVersion);
	switch (options.cmd()) {
	case Common::kCmdDecompile:
		{
			fs::path output;
			if (options.hasOption("output") && !outputIsDirectory) {
				output = options.stringValue("output");
			} else {
				std::string oldExtension = input.extension().string();
				std::string newExtension = (dir->isCast()) ? ".cst" : ".dir";
				std::string fileName = input.stem().string();
				if (Common::compareIgnoreCase(oldExtension, newExtension) == 0) {
					fileName += "_decompiled";
				}
				fileName += newExtension;
				if (options.hasOption("output") && outputIsDirectory) {
					output = options.stringValue("output");
					output /= fileName;
				} else {
					output = input;
					output.replace_filename(fileName);
				}
			}

			dir->config->unprotect();
			dir->parseScripts();
			if (options.hasOption("dump-scripts")) {
				dir->dumpScripts();
			}
			dir->restoreScriptText();
			dir->writeToFile(output);

			std::string fileType = (dir->isCast()) ? "cast" : "movie";
			Common::log(
				"Decompiled " + versionString(version, dir->fverVersionString) + " " + fileType
				+ " " + input.string() + " to " + output.string()
			);
		}
		break;
	case Common::kCmdVersion:
		{
			Common::VersionStyle style = Common::kVersionStyleLong;
			if (options.hasOption("style")) {
				style = (Common::VersionStyle)options.enumValue("style");
			}
			switch (style) {
			case Common::kVersionStyleLong:
				Common::log(versionString(version, dir->fverVersionString));
				break;
			case Common::kVersionStyleShort:
				Common::log(versionNumber(version, dir->fverVersionString));
				break;
			case Common::kVersionStyleInteger:
				Common::log(std::to_string(version));
				break;
			case Common::kVersionStyleInternal:
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
	Common::Options options;
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
					Common::warning(boost::format("Output must be a directory when input is a directory!"));
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
			}
		}
		if (!processFile(input, options, outputIsDirectory))
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
