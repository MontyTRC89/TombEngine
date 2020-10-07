#pragma once
#include <utility>
extern char* malloc_buffer;
extern int malloc_size;
extern char* malloc_ptr;
extern int malloc_free;
extern int malloc_used;

template <typename T,typename ... Args>
T* game_malloc(size_t count = 1,Args&&...args) noexcept {
	return new T[count]{std::forward<Args>(args)...};
}
void init_game_malloc() noexcept;
void game_free(void* ptr) noexcept;