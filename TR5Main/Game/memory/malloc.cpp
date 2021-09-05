#include "framework.h"
#include "malloc.h"

char* malloc_buffer;
int malloc_size;
char* malloc_ptr;
int malloc_free;
int malloc_used;
void init_game_malloc() noexcept
{
}

void game_free(void* ptr) noexcept
{

	delete[] ptr;
}