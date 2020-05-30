#include "framework.h"
#include "malloc.h"


char* malloc_buffer;
int malloc_size;
char* malloc_ptr;
int malloc_free;
int malloc_used;

char* game_malloc(int size)
{
	char* ptr;

	size = (size + 3) & 0xfffffffc;
	if (size <= malloc_free)
	{
		ptr = malloc_ptr;
		malloc_free -= size;
		malloc_used += size;
		malloc_ptr += size;
		return ptr;
	}

	return 0;
}

void init_game_malloc()
{
	malloc_size = 1048576 * 128;
	malloc_buffer = (char*)malloc(malloc_size);
	malloc_ptr = malloc_buffer;
	malloc_free = malloc_size;
	malloc_used = 0;
}

void game_free(int size, int type)
{
	size = (size + 3) & (~3); 
	malloc_ptr -= size;
	malloc_free += size;
	malloc_used -= size;
}