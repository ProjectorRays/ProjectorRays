/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "common/json.h"
#include "common/util.h"

namespace Common {

void JSONWriter::writeString(std::string str) {
	write("\"");
	write(escapeString(str));
	write("\"");
}

void JSONWriter::writeValuePrefix() {
	if (_context == kContextValue) {
		write(",");
	}
	if (_context == kContextValue || _context == kContextOpenBrace) {
		writeLine();
	}
}

void JSONWriter::writeValueSuffix() {
	if (_indentationLevel == 0) {
		writeLine();
	}
}

void JSONWriter::writeCloseBracePrefix() {
	if (_context == kContextValue) {
		writeLine();
	}
}

void JSONWriter::startObject() {
	writeValuePrefix();
	write("{");
	indent();
	_context = kContextOpenBrace;
}

void JSONWriter::writeKey(std::string key) {
	writeValuePrefix();
	writeString(key);
	write(": ");
	_context = kContextKey;
}

void JSONWriter::endObject() {
	writeCloseBracePrefix();
	unindent();
	write("}");
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::startArray() {
	writeValuePrefix();
	write("[");
	indent();
	_context = kContextOpenBrace;
}

void JSONWriter::endArray() {
	writeCloseBracePrefix();
	unindent();
	write("]");
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::writeVal(unsigned int val) {
	writeValuePrefix();
	write(std::to_string(val));
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::writeVal(int val) {
	writeValuePrefix();
	write(std::to_string(val));
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::writeVal(double val) {
	writeValuePrefix();
	write(floatToString(val));
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::writeVal(std::string val) {
	writeValuePrefix();
	writeString(val);
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::writeNull() {
	writeValuePrefix();
	write("null");
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::writeFourCC(uint32_t val) {
	writeValuePrefix();
	write("\"");
	write(fourCCToString(val));
	write("\"");
	_context = kContextValue;
	writeValueSuffix();
}

void JSONWriter::writeField(std::string key, unsigned int val) {
	writeKey(key);
	writeVal(val);
}

void JSONWriter::writeField(std::string key, int val) {
	writeKey(key);
	writeVal(val);
}

void JSONWriter::writeField(std::string key, double val) {
	writeKey(key);
	writeVal(val);
}

void JSONWriter::writeField(std::string key, std::string val) {
	writeKey(key);
	writeVal(val);
}

void JSONWriter::writeNullField(std::string key) {
	writeKey(key);
	writeNull();
}

void JSONWriter::writeFourCCField(std::string key, uint32_t val) {
	writeKey(key);
	writeFourCC(val);
}

std::string JSONWriter::str() const {
	return CodeWriter::str();
}

} // namespace Common
