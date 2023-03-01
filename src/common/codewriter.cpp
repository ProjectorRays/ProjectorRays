/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/codewriter.h"

namespace Common {

void CodeWriter::write(std::string str) {
	if (str.empty())
		return;

	writeIndentation();
	_stream << str;
}

void CodeWriter::write(char ch) {
	writeIndentation();
	_stream << ch;
}

void CodeWriter::writeLine(std::string str) {
	if (str.empty()) {
		_stream << _lineEnding;
	} else {
		writeIndentation();
		_stream << str << _lineEnding;
	}
	_indentationWritten = false;
}

void CodeWriter::writeLine() {
	_stream << _lineEnding;
	_indentationWritten = false;
}

void CodeWriter::indent() {
	_indentationLevel += 1;
}

void CodeWriter::unindent() {
	if (_indentationLevel > 0) {
		_indentationLevel -= 1;
	}
}

std::string CodeWriter::str() const {
	return _stream.str();
}

void CodeWriter::writeIndentation() {
	if (_indentationWritten)
		return;

	for (int i = 0; i < _indentationLevel; i++) {
		_stream << _indentation;
	}

	_indentationWritten = true;
}

} // namespace Common
