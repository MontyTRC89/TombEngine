#include "malloc.h"
#include <stdlib.h>
#include "..\Global\global.h"

/*
void __cdecl InitGameMalloc()
{
	char* buffer = (char*)malloc(GAME_BUFFER_SIZE);
	MallocBuffer = buffer;
	MallocSize = GAME_BUFFER_SIZE;
	MallocPtr = buffer;
	MallocFree = GAME_BUFFER_SIZE;
	MallocUsed = 0;
}

char* __cdecl GameMalloc(__int32 size)
{
	__int32 memSize = (size + 3) & -4;
	if (memSize > MallocFree)
	{
		DB_Log(0, "Out of memory");
		return NULL;
	}
	else
	{
		char* ptr = MallocPtr;
		MallocFree -= memSize;
		MallocUsed += memSize;
		MallocPtr += memSize;
		memset(ptr, 0, 4 * (memSize >> 2));
		return ptr;
	}
}

void __cdecl GameFree(__int32 size)
{
	__int32 memSize = (size + 3) & -4;
	MallocPtr -= memSize;
	MallocFree += memSize;
	MallocUsed -= memSize;
}

void* __cdecl Malloc(__int32 size)
{
	return malloc(size);
}

void __cdecl Free(void* ptr)
{
	free(ptr);
}*/

void Inject_Malloc()
{
	/*INJECT(0x004A7CB0, InitGameMalloc);
	INJECT(0x0040101E, GameMalloc);
	INJECT(0x00403116, GameFree);
	INJECT(0x004E2220, Malloc);
	INJECT(0x004E2C90, Free);*/
}