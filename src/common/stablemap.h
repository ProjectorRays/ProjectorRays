/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_STABLEMAP_H
#define COMMON_STABLEMAP_H

#include <map>
#include "common/util.h"

namespace Common {

template<class Key, class Val, class CompFunc = std::less<Key> >
class StableMap {
public:
	using iterator = typename std::map<Key, Val, CompFunc>::iterator;
	using const_iterator = typename std::map<Key, Val, CompFunc>::const_iterator;
	using value_type = typename std::map<Key, Val, CompFunc>::value_type;

public:
	iterator begin() {
		return _map.begin();
	}

	iterator end() {
		return _map.end();
	}

	const_iterator begin() const {
		return _map.begin();
	}

	const_iterator end() const {
		return _map.end();
	}

	iterator find(const Key &theKey) {
		return _map.find(theKey);
	}

	const_iterator find(const Key &theKey) const {
		return _map.find(theKey);
	}

	Common::Pair<iterator, bool> insert(Common::Pair<Key, Val> val) {
		auto pair = _map.insert(std::pair(val.first, val.second));
		return {pair.first, pair.second};
	}

	Val &operator[](const Key &theKey) {
		return _map[theKey];
	}

private:
	std::map<Key, Val, CompFunc> _map;
};
} // namespace Common

#endif
