#include "framework.h"
#include "malloc.h"
using namespace std;
char* malloc_buffer;
int malloc_size;
char* malloc_ptr;
int malloc_free;
int malloc_used;
Pool<BlockSize::Small>* gameMemory;
void init_game_malloc() noexcept
{
	gameMemory = new Pool<BlockSize::Small>(8*1024*1024);
}

void game_free(void* ptr) noexcept
{
	gameMemory->free(ptr);
}