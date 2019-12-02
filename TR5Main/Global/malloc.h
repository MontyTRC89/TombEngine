#pragma once

#define MallocBuffer				VAR_U_(0x00E4B10C, char*)
#define MallocSize					VAR_U_(0x00E4B058, int)
#define MallocPtr					VAR_U_(0x00E4B0DC, char*)
#define MallocFree					VAR_U_(0x00E4B0F4, int)
#define MallocUsed					VAR_U_(0x00E4B0F0, int)

#define InitGameMalloc ((void (__cdecl*)()) 0x004A7CB0)
#define GameMallocReal ((char* (__cdecl*)(int)) 0x004A7D00)

char* __cdecl GameMalloc(int size);

void Inject_Malloc();