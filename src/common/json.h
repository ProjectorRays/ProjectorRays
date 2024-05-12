/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_JSON_H
#define COMMON_JSON_H

#include "common/codewriter.h"
#include "common/util.h"

namespace Common {

/**
 * JSONWriter writes a non-standard variant of JSON which allows strings with
 * arbitrary character encodings, unlike standard JSON which mandates Unicode.
 * 
 * String literals in this variant may contain:
 * - The standard single-character escape sequences \", \\, \b, \f, \n, \r, \t
 * - The non-standard single-character escape sequence \v
 * - Printable ASCII characters without corresponding single-character escape
 *   sequences
 * - The non-standard hex code escape sequence \xXX
 */

class JSONWriter : protected CodeWriter {
protected:
	enum Context {
		kContextStart,
		kContextOpenBrace,
		kContextKey,
		kContextValue
	};

	Context _context = kContextStart;

public:
	JSONWriter(std::string lineEnding, std::string indentation = "  ")
		: CodeWriter(lineEnding, indentation) {}

	void startObject();
	void writeKey(std::string key);
	void endObject();

	void startArray();
	void endArray();

	void writeVal(unsigned int val);
	void writeVal(int val);
	void writeVal(double val);
	void writeVal(std::string val);
	void writeNull();
	void writeFourCC(uint32_t val);

	void writeField(std::string key, unsigned int val);
	void writeField(std::string key, int val);
	void writeField(std::string key, double val);
	void writeField(std::string key, std::string val);
	void writeNullField(std::string key);
	void writeFourCCField(std::string key, uint32_t val);

	std::string str() const;

protected:
	void writeString(std::string str);
	void writeValuePrefix();
	void writeValueSuffix();
	void writeCloseBracePrefix();
};

#define JSON_WRITE_FIELD(field) \
	do { \
		json.writeKey(#field); \
		json.writeVal((field)); \
	} while (0)

#define JSON_WRITE_FOURCC_FIELD(field) \
	do { \
		json.writeKey(#field); \
		json.writeFourCC((field)); \
	} while (0)

} // namespace Common

#endif // COMMON_JSON_H
