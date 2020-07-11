#pragma once
#include <cstdint>
#include <iterator>
#include <vector>
#include <iostream>
#include "pool.h"
namespace T5M::Memory {
	template <typename T,typename Pool>
	class PoolAllocator {

	public:
		Pool* pool;
		typedef std::size_t     size_type;
		typedef T* pointer;
		typedef T               value_type;

		PoolAllocator(Pool* pool) : pool(pool) {}

		PoolAllocator(const PoolAllocator& other) throw() : pool(other.pool) {};

		template<typename U>
		PoolAllocator(const PoolAllocator<U,Pool>& other) throw() : pool(other.pool) {};

		template<typename U>
		PoolAllocator& operator = (const PoolAllocator<U,Pool>& other) {
			return *this;
		}
		PoolAllocator<T,Pool>& operator = (const PoolAllocator& other) {
			return *this;
		}
		~PoolAllocator() {}

		pointer allocate(size_type n, const void* hint = 0) {
			return pool->malloc<value_type>(n);
		}
		void deallocate(T* ptr, size_type n) {
			pool->free(ptr);
		}

		size_type max_size() const {
#pragma push_macro("max") //thanks Microsoft
#undef max
			return numeric_limits<size_type>::max();
#pragma pop_macro("max")
		}
	};
}
