#include "framework.h"
#include "malloc.h"
#include "door.h"
char* malloc_buffer;
int malloc_size;
char* malloc_ptr;
int malloc_free;
int malloc_used;

constexpr size_t malloc_buffer_size = 1024 * 1024 * 1024; // 1GB
void init_game_malloc() noexcept
{
	malloc_buffer = new char[malloc_buffer_size];
	malloc_ptr = malloc_buffer;
	malloc_size = malloc_buffer_size;
	malloc_free = malloc_size;
	malloc_used = 0;
}

void game_free(void* ptr) noexcept
{
}

void flush_malloc() {
	malloc_ptr = malloc_buffer;
	malloc_free = malloc_size;
}