/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef LINGODEC_RESOLVER_H
#define LINGODEC_RESOLVER_H

#include <cstdint>

namespace LingoDec {

struct Script;
struct ScriptNames;

class ChunkResolver {
public:
	ChunkResolver() {}
	virtual ~ChunkResolver() {}
	virtual Script *getScript(int32_t id) = 0;
	virtual ScriptNames *getScriptNames(int32_t id) = 0;
};

}

#endif // LINGODEC_RESOLVER_H
