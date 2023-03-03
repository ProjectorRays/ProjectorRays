/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <cstring>
#include <string>

#include "common/options.h"
#include "common/log.h"

namespace Common {

Options::Options() {
	addCommand(kCmdDecompile, "decompile", "Unprotect a file and decompile its scripts.");
	addStringOption(false, kCmdDecompile, "output", "Output file. Default is chosen based on the input file's name.", "file", 'o');
	addOption(false, kCmdAll, "dump-scripts", "Dump scripts");

	addCommand(kCmdVersion, "version", "Print the Director version with which the file was created.");
	std::vector<EnumOptionInfo> versionStyles = {
		{ "long",		kVersionStyleLong,		"Version including product name, e.g. \"Macromedia Director 8.0#178\" for 8.0" },
		{ "short",		kVersionStyleShort,		"Version without product name, e.g. \"8.0#178\" for 8.0" },
		{ "integer",	kVersionStyleInteger,	"Version in the style of Lingo's `the fileVersion`, e.g. \"800\" for 8.0" },
		{ "internal",	kVersionStyleInternal,	"Director's internal version number, e.g. \"1600\" for 8.0"}
	};
	addEnumOption(false, kCmdVersion, "style", "Style in which to print the version. Options are:", "name", versionStyles, '\0', "long");

	addOption(true, kCmdAll, "verbose", "Verbose logging", 'v');
	addOption(true, kCmdAll, "dump-chunks", "Dump chunk data");
	addOption(true, kCmdAll, "dump-json", "Dump JSONified chunk data");
};

void Options::addCommand(Command cmd, const char *name, const char *desc) {
	_commandInfo.push_back({ cmd, name, desc });
}

Command Options::getCommand(std::string name) {
	for (const CommandInfo &info : _commandInfo) {
		if (name == info.name)
			return info.cmd;
	}
	return kCmdNone;
}

std::string Options::getCommandName(Command cmd) {
	for (const CommandInfo &info : _commandInfo) {
		if (cmd == info.cmd)
			return info.name;
	}
	return "";
}

std::string Options::getCommandDesc(Command cmd) {
	for (const CommandInfo &info : _commandInfo) {
		if (cmd == info.cmd)
			return info.desc;
	}
	return "";
}

void Options::addOption(bool debug, unsigned int cmd, const char *longName, const char *desc, char shortName) {
	OptionInfo opt;
	opt.debug = debug;
	opt.cmd = cmd;
	opt.longName = longName;
	opt.desc = desc;
	opt.shortName = shortName;
	_optionInfo.push_back(opt);
}

void Options::addStringOption(bool debug, unsigned int cmd, const char *longName, const char *desc, const char *argName, char shortName, const char *def) {
	OptionInfo opt;
	opt.debug = debug;
	opt.cmd = cmd;
	opt.longName = longName;
	opt.desc = desc;
	opt.argName = argName;
	opt.shortName = shortName;
	opt.def = def;
	_optionInfo.push_back(opt);
}

void Options::addEnumOption(bool debug, unsigned int cmd, const char *longName, const char *desc, const char *argName, std::vector<EnumOptionInfo> enumInfo, char shortName, const char *def) {
	OptionInfo opt;
	opt.debug = debug;
	opt.cmd = cmd;
	opt.longName = longName;
	opt.desc = desc;
	opt.argName = argName;
	opt.enumInfo = enumInfo;
	opt.shortName = shortName;
	opt.def = def;
	_optionInfo.push_back(opt);
}

const Options::OptionInfo *Options::getOptionInfo(std::string longName) {
	for (const OptionInfo &info : _optionInfo) {
		if (longName == info.longName) {
			return &info;
		}
	}
	return nullptr;
}

const Options::OptionInfo *Options::getOptionInfo(char shortName) {
	for (const OptionInfo &info : _optionInfo) {
		if (shortName == info.shortName)
			return &info;
	}
	return nullptr;
}

void Options::parse(int argc, char *argv[]) {
	_valid = false;

	_programName.clear();
	_cmd = kCmdNone;
	_inputFile.clear();
	bool inputFileFound = false;

	_optionsNoArg.clear();
	_stringOptions.clear();
	_enumOptions.clear();

	if (argc > 0) {
		_programName = argv[0];
	}
	if (argc > 1) {
		std::string cmdString = argv[1];
		_cmd = getCommand(cmdString);
		if (_cmd == kCmdNone) {
			Common::warning("Unknown command: " + cmdString + "\n");
			printUsage();
			return;
		}
	} else {
		Common::warning("Command not specified\n");
		printUsage();
		return;
	}

	for (int i = 2; i < argc; i++) {
		std::string arg = argv[i];
		if (arg.size() > 0 && arg[0] == '-') {
			if (arg.size() == 1) {
				Common::warning("Unknown option: -\n");
				printUsage();
				return;
			}

			std::string optionString;
			const OptionInfo *info = nullptr;
			bool optionArgFound = false;
			std::string optionArg;

			if (arg.size() > 1 && arg[1] == '-') {
				// Long option
				std::string option = arg.substr(2);

				size_t equalsPos = option.find('=');
				if (equalsPos != std::string::npos) {
					optionArg = option.substr(equalsPos + 1);
					option = option.substr(0, equalsPos);
					optionArgFound = true;
				}

				optionString = "--" + option;
				info = getOptionInfo(option);
				if (!info || !(info->cmd & _cmd)) {
					Common::warning("Unknown option: " + optionString + "\n");
					printUsage();
					return;
				}
			} else {
				// Short option
				for (size_t j = 1; j < arg.size(); j++) {
					char option = arg[j];
					optionString = "-";
					optionString += option;
					info = getOptionInfo(option);
					if (!info || !(info->cmd & _cmd)) {
						Common::warning("Unknown option: " + optionString + "\n");
						printUsage();
						return;
					}
					if (j < arg.size() - 1) {
						if (info->argName) {
							optionArg = arg.substr(j + 1);
							optionArgFound = true;
							break;
						} else {
							_optionsNoArg.insert(info->longName);
						}
					}
				}
			} 
			if (info->argName) {
				if (!optionArgFound) {
					if (i < argc - 1) {
						optionArg = argv[i + 1];
						i++;
					} else {
						Common::warning("Argument not found for " + optionString + "\n");
						printUsage();
						return;
					}
				}
				if (info->enumInfo.size() > 0) {
					bool enumFound = false;
					unsigned int value = 0;
					for (const EnumOptionInfo &enumInfo : info->enumInfo) {
						if (optionArg == enumInfo.name) {
							value = enumInfo.value;
							enumFound = true;
							break;
						}
					}
					if (!enumFound) {
						Common::warning("Invalid argument for " + optionString + ": " + optionArg + "\n");
						printUsage();
						return;
					}
					_enumOptions[info->longName] = value;
				} else {
					_stringOptions[info->longName] = optionArg;
				}
			} else if (optionArgFound) {
				Common::warning("Extra argument for " + optionString + "\": " + optionArg + "\n");
				printUsage();
				return;
			} else {
				_optionsNoArg.insert(info->longName);
			}
		} else if (!inputFileFound) {
			_inputFile = arg;
			inputFileFound = true;
		} else {
			Common::warning("Stray argument: " + arg + "\n");
			printUsage();
			return;
		}
	}

	if (!inputFileFound) {
		Common::warning("Input file not specified\n");
		printUsage();
		return;
	}

	_valid = true;
};

void Options::printUsage() {
	Common::warning("Usage: " + _programName + " <command> <input file> [<option>...]");

	if (_cmd == kCmdNone || _cmd == kCmdAll) {
		Common::warning("\nThe following commands are available:");
	} else {
		Common::warning("\nCommand info:");
	}

	std::vector<std::vector<std::pair<std::string, std::string>>> optionTexts;
	if (_cmd != kCmdNone && _cmd != kCmdAll) {
		optionTexts.push_back(getOptionText(_cmd, false));
	} else {
		for (const CommandInfo &info : _commandInfo) {
			optionTexts.push_back(getOptionText(info.cmd, false));
		}
	}
	optionTexts.push_back(getOptionText(kCmdAll, true));

	size_t leftColWidth = 0;
	for (const auto &optionText : optionTexts) {
		for (const auto &[leftCol, rightCol] : optionText) {
			if (!rightCol.empty()) {
				if (leftCol.size() > leftColWidth) {
					leftColWidth = leftCol.size();
				}
			}
		}
	}
	if (leftColWidth % 4 != 0) {
		leftColWidth += 4 - (leftColWidth % 4);
	}
	leftColWidth += 4;

	for (const auto &optionText : optionTexts) {
		Common::warning("");
		for (const auto &[leftCol, rightCol] : optionText) {
			std::string line = leftCol;
			if (!rightCol.empty()) {
				while (line.size() < leftColWidth) {
					line += " ";
				}
				line += rightCol;
			}
			Common::warning(line);
		}
	}
}

std::vector<std::pair<std::string, std::string>> Options::getOptionText(Command cmd, bool debug) {
	std::vector<std::pair<std::string, std::string>> res;
	if (cmd != kCmdNone && cmd != kCmdAll) {
		res.push_back(std::make_pair(getCommandName(cmd) + " <input file>", getCommandDesc(cmd)));
	} else if (debug) {
		res.push_back(std::make_pair("Debug options:", ""));
	}
	for (const OptionInfo &info : _optionInfo) {
		if (!(info.cmd & cmd) || info.debug != debug)
			continue;
		
		std::string left = "    ";
		if (info.shortName) {
			left += "-";
			left += info.shortName;
			left += ", ";
		}
		left += "--";
		left += info.longName;
		if (info.argName) {
			left += " <";
			left += info.argName;
			left += ">";
		}
		
		std::string right = "    ";
		right += info.desc;

		res.push_back(std::make_pair(left, right));

		if (info.enumInfo.size() > 0) {
			for (const EnumOptionInfo &enumInfo : info.enumInfo) {
				std::string enumLeft = "         ";
				enumLeft += enumInfo.name;
				if (std::strcmp(enumInfo.name, info.def) == 0) {
					enumLeft += " (default)";
				}
				std::string enumRight = "        ";
				enumRight += enumInfo.desc;
				res.push_back(std::make_pair(enumLeft, enumRight));
			}
		}
	}
	return res;
}

} // namespace Common
