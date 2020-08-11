#pragma once
#include "pool.h"
#include <utility>
#include "memory/memory.h"
extern char* malloc_buffer;
extern int malloc_size;
extern char* malloc_ptr;
extern int malloc_free;
extern int malloc_used;
extern T5M::Memory::TGPool* gameMemory;

template <typename T,typename ... Args>
[[nodiscard]] T* game_malloc(size_t count = 1,Args&&...args) noexcept {
#if CUSTOM_MEMORY
	return gameMemory->malloc<T>(count,std::forward<Args>(args)...);
#else
	return new T[count]{std::forward<Args>(args)...};
#endif
}
void init_game_malloc() noexcept;
void game_free(void* ptr) noexcept;