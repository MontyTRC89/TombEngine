#pragma once
#include "pool.h"
#include "PoolAllocator.h"
#include <vector>
namespace T5M::Memory {
	using TGPool = Pool<BlockSize::Medium>;
	template<typename T> using vector = std::vector <T, PoolAllocator<T, TGPool>>;
}