#pragma once
#include <utility>
extern char* malloc_ptr;
extern int malloc_free;
extern int malloc_used;



template <typename T,typename ... Args>
T* game_malloc(size_t count = 1,Args&&...args) noexcept {

	size_t requestedSize = sizeof(T) * count;
	if(requestedSize > malloc_free)
	{
		return nullptr;
	}
	T* ptr = new (malloc_ptr)T(std::forward<Args>(args)...);
	malloc_used += requestedSize;
	malloc_free -= requestedSize;
	malloc_ptr += requestedSize;
	return ptr;
}

void flush_malloc();
void init_game_malloc() noexcept;
void game_free(void* ptr) noexcept;