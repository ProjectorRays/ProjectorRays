/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "common/options.h"
#include "common/fileio.h"
#include "common/log.h"
#include "common/stream.h"
#include "common/util.h"
#include "director/chunk.h"
#include "director/dirfile.h"
#include "director/util.h"

using namespace Director;

int main(int argc, char *argv[]) {
	Common::Options options;
	options.parse(argc, argv);
	if (!options.valid()) {
		return EXIT_FAILURE;
	}
	if (options.hasOption("verbose")) {
		Common::g_verbose = true;
	}

	std::filesystem::path input = options.inputFile();
	std::vector<uint8_t> buf;
	if (!Common::readFile(input, buf)) {
		Common::warning(boost::format("Could not read %s!") % input);
		return EXIT_FAILURE;
	}

	Common::ReadStream stream(buf.data(), buf.size());
	auto dir = std::make_unique<DirectorFile>();
	if (!dir->read(&stream))
		return EXIT_FAILURE;

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
			std::filesystem::path output;
			if (options.hasOption("output")) {
				output = options.stringValue("output");
			} else {
				std::string oldExtension = input.extension().string();
				std::string newExtension = (dir->isCast()) ? ".cst" : ".dir";
				std::string fileName = input.stem().string();
				if (Common::compareIgnoreCase(oldExtension, newExtension) == 0) {
					fileName += "_decompiled";
				}
				fileName += newExtension;
				output = input;
				output.replace_filename(fileName);
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

	return EXIT_SUCCESS;
}
