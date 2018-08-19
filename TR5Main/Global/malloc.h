#pragma once

#define MallocBuffer				VAR_U_(0x00E4B10C, char*)
#define MallocSize					VAR_U_(0x00E4B058, __int32)
#define MallocPtr					VAR_U_(0x00E4B0DC, char*)
#define MallocFree					VAR_U_(0x00E4B0F4, __int32)
#define MallocUsed					VAR_U_(0x00E4B0F0, __int32)

#define InitGameMalloc ((void (__cdecl*)()) 0x004A7CB0)
#define GameMalloc ((char* (__cdecl*)(int)) 0x004A7D00)

void Inject_Malloc();