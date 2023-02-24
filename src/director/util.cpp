/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "director/lingo.h"
#include "director/util.h"

namespace Director {

std::string indent(std::string str) {
	std::string res;
	size_t pos = str.find(kLingoLineEnding);
	while (pos != std::string::npos) {
		res += "  " + str.substr(0, pos + 1);
		str = str.substr(pos + 1);
		pos = str.find(kLingoLineEnding);
	}
	return res;
}

unsigned int humanVersion(unsigned int ver) {
	// This is based on Lingo's `the fileVersion` with a correction to the
	// version number for Director 12.
	if (ver >= 1951)
		return 1200;
	if (ver >= 1922)
		return 1150;
	if (ver >= 1921)
		return 1100;
	if (ver >= 1851)
		return 1000;
	if (ver >= 1700)
		return 850;
	if (ver >= 1410)
		return 800;
	if (ver >= 1224)
		return 700;
	if (ver >= 1218)
		return 600;
	if (ver >= 1201)
		return 500;
	if (ver >= 1117)
		return 404;
	if (ver >= 1115)
		return 400;
	if (ver >= 1029)
		return 310;
	if (ver >= 1028)
		return 300;
	return 200;
}

std::string versionNumber(unsigned int ver, const std::string &fverVersionString) {
	unsigned int major = ver / 100;
	unsigned int minor = (ver / 10) % 10;
	unsigned int patch = ver % 10;

	std::string versionNumber;
	if (fverVersionString.empty()) {
		versionNumber = std::to_string(major) + "." + std::to_string(minor);
		if (patch)
			versionNumber += "." + std::to_string(patch);
	} else {
		versionNumber = fverVersionString;
	}
	return versionNumber;
}

std::string versionString(unsigned int ver, const std::string &fverVersionString) {
	unsigned int major = ver / 100;
	std::string versionNum = versionNumber(ver, fverVersionString);

	if (major >= 11)
		return "Adobe Director " + versionNum;

	if (major == 10)
		return "Macromedia Director MX 2004 (" + versionNum + ")";

	if (major == 9)
		return "Macromedia Director MX (" + versionNum + ")";

	return "Macromedia Director " + versionNum;
}

} // namespace Director
