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

class CodeWriter {
protected:
	std::stringstream _stream;
	int _indentationLevel = 0;
	bool _indentationWritten = false;

public:
	void write(std::string str);
	void write(char ch);
	void writeLine(std::string str);
	void writeLine();

	void indent();
	void unindent();

	std::string str() const;

protected:
	void writeIndentation();
};

} // namespace Common

#endif // COMMON_CODEWRITER_H
