#include "patch.h"
#include "..\Global\global.h"1
#include <stdio.h>

// Remapped variables. Some variables must be moved from EXE space to DLL space for having free space for bigger arrays
__int32 NumItems;
__int32 dword_874254;
__int32 unk_87435C;
ITEM_INFO* Targets[NUM_SLOTS];
STATIC_INFO StaticObjects[NUM_STATICS];

void PatchGameCode()
{
	printf("Patching game code\n");

	HANDLE gameHandle = GetCurrentProcess();

	// Remap some variables for more slots
	__int32 *newAddress = &NumItems;
	WriteProcessMemory(gameHandle, (LPVOID)0x4A63A5, &newAddress, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A63CA, &newAddress, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A63E3, &newAddress, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A64C3, &newAddress, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A64DC, &newAddress, 4, NULL);

	newAddress = &nAnimUVRanges;
	WriteProcessMemory(gameHandle, (LPVOID)0x4A51C2, &newAddress, 4, NULL);

	newAddress = &dword_874254;
	WriteProcessMemory(gameHandle, (LPVOID)0x4A6783, &newAddress, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4C55A4, &newAddress, 4, NULL);

	newAddress = &unk_87435C;
	WriteProcessMemory(gameHandle, (LPVOID)0x4A6AE3, &newAddress, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4B844E, &newAddress, 4, NULL);

	// Bigger game buffer
	__int32 newValue = GAME_BUFFER_SIZE;
	WriteProcessMemory(gameHandle, (LPVOID)0x4A7CB1, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A7CC8, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A7CD7, &newValue, 4, NULL);

	// More enemies active at the same time
	newValue = NUM_SLOTS;
	WriteProcessMemory(gameHandle, (LPVOID)0x452F49, &newValue, 4, NULL);

	ITEM_INFO** newAddressTargets = &Targets[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x4530A0, &newAddressTargets, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4530F6, &newAddressTargets, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4530FB, &newAddressTargets, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x453115, &newAddressTargets, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x453173, &newAddressTargets, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4531B1, &newAddressTargets, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x45320F, &newAddressTargets, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x453245, &newAddressTargets, 4, NULL);

	// Draw distance
	float newDrawingDistance = DRAW_DISTANCE;
	WriteProcessMemory(gameHandle, (LPVOID)0x511A5C, &newDrawingDistance, 4, NULL);

	// Remove vertex limit in meshes
	__int16 newValue16 = 0;
	//WriteProcessMemory(gameHandle, (LPVOID)0x49A4E1, &newValue16, 2, NULL);

	// More statics
	newValue = (__int32)&StaticObjects[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x41D736, &newValue, 4, NULL); // ShatterObject
	WriteProcessMemory(gameHandle, (LPVOID)0x4A5BDA, &newValue, 4, NULL); // LoadSprites
	WriteProcessMemory(gameHandle, (LPVOID)0x41453F, &newValue, 4, NULL); // TestObjectOnLedge

	newValue = (__int32)&StaticObjects[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x42D185, &newValue, 4, NULL); // sub_42D060
	newValue = (__int32)&StaticObjects[0] + 2;
	WriteProcessMemory(gameHandle, (LPVOID)0x42D12E, &newValue, 4, NULL); // sub_42D060
	newValue = (__int32)&StaticObjects[0] + 4;
	WriteProcessMemory(gameHandle, (LPVOID)0x42D117, &newValue, 4, NULL); // sub_42D060

	newValue = (__int32)&StaticObjects[0] + 16;
	WriteProcessMemory(gameHandle, (LPVOID)0x4687D0, &newValue, 4, NULL); // sub_468770
	newValue = (__int32)&StaticObjects[0] + 18;
	WriteProcessMemory(gameHandle, (LPVOID)0x4687D8, &newValue, 4, NULL); // sub_468770

	newValue = (__int32)&StaticObjects[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x43453F, &newValue, 4, NULL); //  sub_434390

	newValue = (__int32)&StaticObjects[0] + 16;
	WriteProcessMemory(gameHandle, (LPVOID)0x4123AB, &newValue, 4, NULL); // LaraBaddieCollision

	newValue = (__int32)&StaticObjects[0] + 20;
	WriteProcessMemory(gameHandle, (LPVOID)0x413DF0, &newValue, 4, NULL); // sub_413CF0
	newValue = (__int32)&StaticObjects[0] + 16;
	WriteProcessMemory(gameHandle, (LPVOID)0x413DF7, &newValue, 4, NULL); // sub_413CF0

	newValue = (__int32)&StaticObjects[0] + 16;
	WriteProcessMemory(gameHandle, (LPVOID)0x480128, &newValue, 4, NULL); // sub_47FFE0	

	newValue = (__int32)&StaticObjects[0] + 16;
	WriteProcessMemory(gameHandle, (LPVOID)0x4192B0, &newValue, 4, NULL); // ObjectOnLOS2	

	//newValue = 0xFF;
	//WriteProcessMemory(gameHandle, (LPVOID)0x004A7385, &newValue, 4, NULL); // ObjectOnLOS2	
}