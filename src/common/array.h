/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_ARRAY_H
#define COMMON_ARRAY_H

#include <assert.h>
#include <vector>

namespace Common {

template<class T>
class Array {
public:
	typedef typename std::vector<T>::size_type size_type;
	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;

public:
	Array() {}
	explicit Array(size_type count) : _arr(count) {}

	size_type size() const {
		return _arr.size();
	}

	T &operator[](size_type idx) {
		assert(idx < size());
		return _arr[idx];
	}

	const T &operator[](size_type idx) const {
		assert(idx < size());
		return _arr[idx];
	}

	void push_back(const T &element) {
		_arr.push_back(element);
	}

	void pop_back() {
		return _arr.pop_back();
	}

	iterator erase(iterator pos) {
		return _arr.erase(pos);
	}

	void resize(size_type newSize) { _arr.resize(newSize); }

	bool empty() const {
		return _arr.empty();
	}

	void clear() {
		_arr.clear();
	}

	iterator begin() {
		return _arr.begin();
	}

	iterator end() {
		return _arr.end();
	}

	const_iterator begin() const {
		return _arr.begin();
	}

	const_iterator end() const {
		return _arr.end();
	}

	T &front() {
		return _arr.front();
	}

	const T &front() const {
		return _arr.front();
	}

	T &back() {
		return _arr.back();
	}

	const T &back() const {
		return _arr.back();
	}

private:
	std::vector<T> _arr;
};

} // namespace Common

#endif