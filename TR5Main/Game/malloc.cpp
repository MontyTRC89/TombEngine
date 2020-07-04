#include "framework.h"
#include "malloc.h"
#include "qmalloc.h"
#if REPLACE_HEAP_MEMORY
#include <new>
#endif
char* malloc_buffer;
int malloc_size;
char* malloc_ptr;
int malloc_free;
int malloc_used;

void* game_malloc(int size)
{
	return T5M::Memory::malloc(size);
}

void init_game_malloc()
{
}

void game_free(void* ptr)
{
	T5M::Memory::free(ptr);
}

#if REPLACE_HEAP_MEMORY
void* operator new(size_t sz) {
#if _DEBUG_MEM
	std::printf("global op new called, size = %zu\n", sz);
#endif
	void* ptr = game_malloc(sz);
	if (ptr)
		return ptr;
	else
		throw std::bad_alloc{};
}
void operator delete(void* ptr) noexcept {
#if _DEBUG_MEM
	std::puts("global op delete called");
#endif
	game_free(ptr);
}
#endif