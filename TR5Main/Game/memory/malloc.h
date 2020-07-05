#pragma once

extern char* malloc_buffer;
extern int malloc_size;
extern char* malloc_ptr;
extern int malloc_free;
extern int malloc_used;

void* game_malloc(int size);
void init_game_malloc();
void game_free(void* ptr);