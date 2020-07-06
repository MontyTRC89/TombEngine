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
using namespace T5M::Memory;
MemoryPool* gameMemory;

void init_game_malloc() noexcept
{
	gameMemory = new MemoryPool(MemoryUnit::MebiByte, 196, 128);
}

void game_free(void* ptr) noexcept
{
	gameMemory->free(ptr);
}