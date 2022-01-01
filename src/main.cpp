/**
 * ProjectorRays Shockwave Decompiler
 * Copyright (C) 2017 Anthony Kleine, Deborah Servilla
 * Copyright (C) 2020-2021 Deborah Servilla
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>
#include <vector>

#include "common/fileio.h"
#include "common/log.h"
#include "common/stream.h"
#include "director/dirfile.h"

using namespace Director;

int main(int argc, char *argv[]) {
	bool decompile = true;
	bool dumpChunks = false;
	bool dumpJSON = false;
	std::string fileName;
	bool foundFileName = false;

	int argsUsed;
	for (argsUsed = 1; argsUsed < argc; argsUsed++) {
		std::string arg = argv[argsUsed];
		if (arg == "--dump-chunks") {
			dumpChunks = true;
		} else if (arg == "--dump-json") {
			dumpJSON = true;
		} else if (arg == "--no-decompile") {
			decompile = false;
		} else if (arg == "-v" || arg == "--verbose") {
			Common::g_verbose = true;
		} else if (!foundFileName) {
			fileName = arg;
			foundFileName = true;
		} else {
			break;
		}
	}

	if (argsUsed != argc || !foundFileName) {
		Common::log(boost::format("Usage: %s [OPTIONS]... FILE") % argv[0]);
		Common::log("  --dump-chunks\t\tDump chunk data");
		Common::log("  --dump-json\t\tDump JSONifed chunk data");
		Common::log("  --no-decompile\tDon't decompile Lingo");
		Common::log("  -v or --verbose\tVerbose logging");
		return 1;
	}

	auto buf = Common::readFile(fileName);
	auto stream = std::make_unique<Common::ReadStream>(buf);
	auto dir = std::make_unique<DirectorFile>();
	dir->read(stream.get(), decompile);

	if (decompile) {
		dir->dumpScripts();
	}
	if (dumpChunks) {
		dir->dumpChunks();
	}
	if (dumpJSON) {
		dir->dumpJSON();
	}

	dir->restoreScriptText();
	dir->writeToFile("test.dir");

	return 0;
}
