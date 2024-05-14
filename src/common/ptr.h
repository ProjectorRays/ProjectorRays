/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef COMMON_PTR_H
#define COMMON_PTR_H

#include <memory>

namespace Common {

template<class T>
class SharedPtr {
public:
	typedef T *PointerType;
	typedef T &ReferenceType;
	template<class T2>
	friend class SharedPtr;

public:
	SharedPtr() {}
	SharedPtr(T *ptr) : _ptr(ptr) {}

	SharedPtr(std::nullptr_t) : _ptr(nullptr) {
	}

	template<class T2>
	SharedPtr(const SharedPtr<T2> &r) : _ptr(r._ptr) {
	}

	T *operator->() const { return _ptr.operator->(); }

	operator bool() const {
		return _ptr.operator bool();
	}

	operator bool() {
		return _ptr.operator bool();
	}

	PointerType get() const { return _ptr.get(); }

	ReferenceType operator*() const { return *_ptr; }

private:
	std::shared_ptr<T> _ptr;
};

template<class T>
class ScopedPtr {
public:
	typedef T *PointerType;
	typedef T &ReferenceType;

public:
	explicit ScopedPtr(PointerType o = nullptr) : _ptr(o) {}

	ScopedPtr(std::nullptr_t) : _ptr(nullptr) {}

	template<class T2>
	ScopedPtr(ScopedPtr<T2> &&o) : _ptr(o._ptr) {
		o._pointer = nullptr;
	}

	template<class T2>
	ScopedPtr &operator=(ScopedPtr<T2> &&other) {
		_ptr = other._ptr;
		return *this;
	}

	ReferenceType operator*() const { return *_ptr; }
	PointerType operator->() const { return _ptr.operator->(); }
	PointerType get() const { return _ptr.get(); }

private:
	std::unique_ptr<T> _ptr;
};

} // namespace Common

#endif