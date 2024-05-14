/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include <cstdint>
#include <string>
#include "common/str.h"
#include "common/log.h"

#define FOURCC(a0, a1, a2, a3) ((uint32_t)((a3) | ((a2) << 8) | ((a1) << 16) | ((a0) << 24)))

#define STR_INNER(x) #x
#define STR(x) STR_INNER(x)
#define SWAP(a, b) std::swap((a),(b))

#define toPrintable(s) escapeString(s)

namespace Common {

std::string fourCCToString(uint32_t fourcc);
std::string floatToString(double f);
std::string byteToString(uint8_t byte);
std::string escapeString(const char *str, size_t size);
std::string escapeString(Common::String str);
int stricmp(const char *a, const char *b);
int compareIgnoreCase(const std::string &a, const std::string &b);

template<class T>
constexpr std::remove_reference_t<T> &&move(T &&t) noexcept {
	return static_cast<std::remove_reference_t<T> &&>(t);
}

template<class T1, class T2>
struct Pair {
	T1 first;
	T2 second;

	Pair() {}
	Pair(T1 first_, T2 second_) : first(first_), second(second_) {
	}
};

} // namespace Common

void warning(const Common::String &msg);


#endif // COMMON_UTIL_H
