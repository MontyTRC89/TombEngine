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
	std::vector<int*> ptrVec;
	for (int i = 0; i < 100; i++) {
		int* j = gameMemory->malloc<int>(1, i);
		ptrVec.push_back(j);
	}
	for (int i = 0; i < 100; i++) {
		gameMemory->free(ptrVec[0]);
		ptrVec.erase(ptrVec.begin());
	}
	for (int i = 0; i < 100; i++) {
		int* j = gameMemory->malloc<int>(1, i);
		ptrVec.push_back(j);
	}

	gameMemory->free(ptrVec[50]);
	gameMemory->free(ptrVec[51]);
	gameMemory->free(ptrVec[52]);
	ptrVec.push_back(gameMemory->malloc<int>(1, 300));
	ptrVec.push_back(gameMemory->malloc<int>(1, 301));
	ptrVec.push_back(gameMemory->malloc<int>(1, 302));

	for (auto i = ptrVec.begin(); i != ptrVec.end() ; i++) {
		std::cout << **i << std::endl;
	}
}

void game_free(void* ptr) noexcept
{
	gameMemory->free(ptr);
}