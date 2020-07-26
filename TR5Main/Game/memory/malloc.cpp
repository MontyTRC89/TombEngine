#include "framework.h"
#include "malloc.h"
#include "door.h"
#include "PoolAllocator.h"
using namespace T5M::Memory;
char* malloc_buffer;
int malloc_size;
char* malloc_ptr;
int malloc_free;
int malloc_used;
TGPool* gameMemory;
void init_game_malloc() noexcept
{
	gameMemory = new TGPool(8 * 1024 * 1024);
}

void game_free(void* ptr) noexcept
{
	gameMemory->free(ptr);
}