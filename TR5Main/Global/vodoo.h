#pragma once

#include <Windows.h>

#pragma pack(push, 1)
typedef struct {
	BYTE opCode;	// must be 0xE9;
	DWORD offset;	// jump offset
} JMP;
#pragma pack(pop)

#define INJECT(from,to) { \
	((JMP*)(from))->opCode = 0xE9; \
	((JMP*)(from))->offset = (DWORD)(to) - ((DWORD)(from) + sizeof(JMP)); \
}

#define VAR_U_(address, type)			(*(type*)(address)) // uninitialized variable
#define VAR_I_(address, type, value)	(*(type*)(address)) // initialized variable (value is just for info)
#define ARRAY_(address, type, length)	(*(type(*)length)(address)) // array (can be multidimensional)
