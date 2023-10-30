/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_OPTIONS_H
#define COMMON_OPTIONS_H

#include <cstdio>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Common {

enum Command {
	kCmdNone		= 0,
	kCmdDecompile	= (1 << 0),
	kCmdVersion		= (1 << 1),
	kCmdAll			= (1 << 2) - 1
};

enum VersionStyle {
	kVersionStyleLong,
	kVersionStyleShort,
	kVersionStyleInteger,
	kVersionStyleInternal
};

class Options {
private:
	struct CommandInfo {
		Command cmd;
		const char *name;
		const char *desc;
	};

	struct EnumOptionInfo {
		const char *name;
		unsigned int value;
		const char *desc;
	};

	struct OptionInfo {
		bool debug = false;
		unsigned int cmd = kCmdNone;
		const char *longName = nullptr;
		const char *desc = nullptr;
		std::vector<EnumOptionInfo> enumInfo;
		char shortName = '\0';
		const char *argName = nullptr;
		const char *def = nullptr;
	};

	std::vector<CommandInfo> _commandInfo;
	std::vector<OptionInfo> _optionInfo;

	bool _valid = false;

	std::string _programName;
	Command _cmd = kCmdNone;
	std::string _inputFile;

	std::set<std::string> _optionsNoArg;
	std::map<std::string, std::string> _stringOptions;
	std::map<std::string, unsigned int> _enumOptions;

	void addCommand(Command cmd, const char *name, const char *desc);
	Command getCommand(std::string name);
	std::string getCommandName(Command cmd);
	std::string getCommandDesc(Command cmd);

	void addOption(bool debug, unsigned int cmd, const char *longName, const char *desc, char shortName = '\0');
	void addStringOption(bool debug, unsigned int cmd, const char *longName, const char *desc, const char *argName, char shortName = '\0', const char *def = nullptr);
	void addEnumOption(bool debug, unsigned int cmd, const char *longName, const char *desc, const char *argName, std::vector<EnumOptionInfo> enumInfo, char shortName = '\0', const char *def = nullptr);
	const OptionInfo *getOptionInfo(std::string longName);
	const OptionInfo *getOptionInfo(char shortName);

	void printUsage(FILE *fh = stderr);
	std::vector<std::pair<std::string, std::string>> getOptionText(Command cmd, bool debug);

public:
	Options();

	void parse(int argc, char *argv[]);

	bool valid() const { return _valid; }
	Command cmd() const { return _cmd; }
	std::string inputFile() const { return _inputFile; }
	bool hasOption(std::string option) const { return _optionsNoArg.count(option) || _stringOptions.count(option) || _enumOptions.count(option); }
	bool hasDumpOptions() const;
	bool hasCastDumpOptions() const;
	bool hasChunkDumpOptions() const;
	std::string stringValue(std::string option) const { return _stringOptions.at(option); }
	unsigned int enumValue(std::string option) const { return _enumOptions.at(option); }
};

} // namespace Common

#endif // COMMON_OPTIONS_H
