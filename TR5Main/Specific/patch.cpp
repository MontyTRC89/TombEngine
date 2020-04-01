#include "patch.h"
#include "..\Global\global.h"
#include <stdio.h>
#include "roomload.h"
#include "..\Game\larafire.h"

void PatchGame()
{
	printf("Patching game code\n");

	HANDLE gameHandle = GetCurrentProcess();

	// Bigger game buffer
	int newValue = GAME_BUFFER_SIZE * 2;
	WriteProcessMemory(gameHandle, (LPVOID)0x4A7CB1, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A7CC8, &newValue, 4, NULL);
	WriteProcessMemory(gameHandle, (LPVOID)0x4A7CD7, &newValue, 4, NULL);

	// Move LevelDataPtr for having more space for Objects[]
	newValue = (int)&LevelDataPtr;
	
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

	// InitialiseLara
	newValue = (int)&Weapons[0] + 108;
	WriteProcessMemory(gameHandle, (LPVOID)0x00473442, &newValue, 4, NULL);
}