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
MemoryPool* gameMemory = nullptr;
void* game_malloc(int size)
{
	return gameMemory->malloc<uint8_t>(size);
}

void init_game_malloc()
{
	gameMemory = new MemoryPool(MemoryUnit::MebiByte, 512, 256);
}

void game_free(void* ptr)
{
	gameMemory->free(ptr);
}