#include "patch.h"
#include "..\Global\global.h"
#include <stdio.h>
#include "roomload.h"
#include "..\Game\larafire.h"

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

	// Move LevelDataPtr for having more space for Objects[]
	newValue = (__int32)&LevelDataPtr;
	
	// LoadRooms
	WriteProcessMemory(gameHandle, (LPVOID)0x004916C4, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004916CE, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004916DA, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x00491706, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x00491710, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x0049171B, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x00491726, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x00491745, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x0049175B, &newValue, 4, NULL);

	// LoadRooms
	WriteProcessMemory(gameHandle, (LPVOID)0x004A4DCF, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A4DDA, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A4DEA, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A4E0C, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A4E1C, &newValue, 4, NULL);

	// LoadSprites
	WriteProcessMemory(gameHandle, (LPVOID)0x004A59E2, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A59EC, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A59F6, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5A20, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5A4B, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5B61, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5B6E, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5B8C, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5B9D, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5BBF, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5BE4, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5BF7, &newValue, 4, NULL);

	// LoadSoundEffects
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5D9D, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5DB5, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5DE2, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5E08, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5E13, &newValue, 4, NULL);

	// LoadBoxes
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5E62, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5E6E, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5E8D, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5EB2, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5EBA, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5EC4, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5ED3, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5EF7, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5F04, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5F26, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5F4D, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5F5C, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5F79, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5F9F, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A5FAE, &newValue, 4, NULL);

	//LoadAnimatedTextures
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6061, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6070, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A607F, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A609E, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A60A7, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A60B3, &newValue, 4, NULL);

	// LoadTextureInfos
	WriteProcessMemory(gameHandle, (LPVOID)0x004A60F1, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A60F9, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A610B, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A613D, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6159, &newValue, 4, NULL);

	// Sub_4A6760
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6761, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6771, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A677C, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A679B, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A67A4, &newValue, 4, NULL);

	// LoadDemoData
	WriteProcessMemory(gameHandle, (LPVOID)0x004A67D1, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A67D9, &newValue, 4, NULL);

	// LoadAIObjects
	WriteProcessMemory(gameHandle, (LPVOID)0x004A67F2, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A67FF, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A681E, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A683D, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6846, &newValue, 4, NULL);

	// LoadSamples
	WriteProcessMemory(gameHandle, (LPVOID)0x004A689F, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A68B2, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A68BC, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A68D4, &newValue, 4, NULL);	
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6913, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6939, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A6941, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x004A694D, &newValue, 4, NULL);

	// TODO: this is a temp patch for weapons
	
	// GetTargetOnLOS
	newValue = (__int32)&Weapons[0] + 32;
	WriteProcessMemory(gameHandle, (LPVOID)0x0041A455, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x0041A4C7, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x0041A525, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x0041A64D, &newValue, 4, NULL);

	// RifleHandler
	newValue = (__int32)&Weapons[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x0044DCDA, &newValue, 4, NULL);

	// FireShotgun
	newValue = (__int32)&Weapons[0] + 186;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044E2B2, &newValue, 4, NULL);
	newValue = (__int32)&Weapons[0] + 188;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044E2D2, &newValue, 4, NULL);

	// FireHK
	newValue = (__int32)&Weapons[0] + 216;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044E3FB, &newValue, 4, NULL);
	newValue = (__int32)&Weapons[0] + 222;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044E403, &newValue, 4, NULL);
	newValue = (__int32)&Weapons[0] + 216;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044E40D, &newValue, 4, NULL);
	newValue = (__int32)&Weapons[0] + 222;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044E415, &newValue, 4, NULL);
	newValue = (__int32)&Weapons[0] + 224;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044E45B, &newValue, 4, NULL);

	// draw_shotgun
	newValue = (__int32)&Weapons[0] + 23;
	WriteProcessMemory(gameHandle, (LPVOID)0x0044EBBB, &newValue, 4, NULL);

	// PistolsHandler
	newValue = (__int32)&Weapons[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x0044FFD2, &newValue, 4, NULL);

	// AnimatePistols
	newValue = (__int32)&Weapons[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x0045044A, &newValue, 4, NULL);

	// LaraGun
	newValue = (__int32)&Weapons[0] + 28;
	WriteProcessMemory(gameHandle, (LPVOID)0x00452564, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x0045265C, &newValue, 4, NULL);

	// FireWeapon
	newValue = (__int32)&Weapons[0];
	WriteProcessMemory(gameHandle, (LPVOID)0x004535E3, &newValue, 4, NULL);

	// InitialiseLara
	newValue = (__int32)&Weapons[0] + 108;
	WriteProcessMemory(gameHandle, (LPVOID)0x00473442, &newValue, 4, NULL);
}