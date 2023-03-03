/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_CODEWRITER_H
#define COMMON_CODEWRITER_H

#include <string>
#include <sstream>

namespace Common {

#ifdef _WIN32
static const char *kPlatformLineEnding = "\r\n";
#else
static const char *kPlatformLineEnding = "\n";
#endif

class CodeWriter {
protected:
	std::stringstream _stream;

	std::string _lineEnding;
	std::string _indentation;

	int _indentationLevel = 0;
	bool _indentationWritten = false;
	size_t _lineWidth = 0;
	size_t _size = 0;

public:
	bool doIndentation = true;

public:
	CodeWriter(std::string lineEnding = kPlatformLineEnding, std::string indentation = "  ")
		: _lineEnding(lineEnding), _indentation(indentation) {}

	void write(std::string str);
	void write(char ch);
	void writeLine(std::string str);
	void writeLine();

	void indent();
	void unindent();

	std::string str() const;
	size_t lineWidth() const { return _lineWidth; }
	size_t size() const { return _size; }

protected:
	void writeIndentation();
};

} // namespace Common

#endif // COMMON_CODEWRITER_H
