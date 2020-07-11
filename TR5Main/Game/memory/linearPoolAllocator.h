#pragma once
#include <cstdint>
#include <iterator>
#include <vector>
#include <iostream>
#include "linearPool.h"
template <typename T>
class LinearPoolAllocator {
private:
	LinearPool* pool;

public:
	typedef std::size_t     size_type;
	typedef T* pointer;
	typedef T               value_type;

	LinearPoolAllocator(LinearPool* pool) : pool(pool){}

	LinearPoolAllocator(const LinearPoolAllocator& other) throw() : pool(other.pool) {};

	template<typename U>
	LinearPoolAllocator(const LinearPoolAllocator<U>& other) throw() : pool(other.pool) {};

	template<typename U>
	LinearPoolAllocator& operator = (const LinearPoolAllocator<U>& other) {
		return *this;
	}
	LinearPoolAllocator<T>& operator = (const LinearPoolAllocator& other) {
		return *this;
	}
	~LinearPoolAllocator() {}

	pointer allocate(size_type n, const void* hint = 0) {
		return pool->malloc<value_type>(n);
	}
	void deallocate(T* ptr, size_type n) {}

	size_type max_size() const {
		return pool->size();
	}
};