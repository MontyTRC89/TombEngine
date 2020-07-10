#pragma once
#include "pool.h"
extern char* malloc_buffer;
extern int malloc_size;
extern char* malloc_ptr;
extern int malloc_free;
extern int malloc_used;
using namespace T5M::Memory;
extern Pool<BlockSize::Small>* gameMemory;
template <typename T>
[[nodiscard]] T* game_malloc(size_t count= 1) noexcept {
	return gameMemory->malloc<T>(count);
}
void init_game_malloc() noexcept;
void game_free(void* ptr) noexcept;