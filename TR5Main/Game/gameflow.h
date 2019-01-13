#pragma once
#include <vector>
#include <string>
#include "savegame.h"

using namespace std;

typedef struct SavegameInfo {
	bool present;
	char levelName[75];
	__int32 saveNumber;
	__int16 days;
	__int16 hours;
	__int16 minutes;
	__int16 seconds;
	char fileName[255];
};

#define MAX_SAVEGAMES 16

extern SavegameInfo g_SavegameInfos[MAX_SAVEGAMES];
extern vector<string> g_NewStrings;
extern SaveGameHeader g_NewSavegameInfos[MAX_SAVEGAMES];

//#define DoGameflow ((__int32 (__cdecl*)(void)) 0x004A8570)
#define LoadGameflow ((__int32 (__cdecl*)(void)) 0x00434800)

__int32 __cdecl LoadSavegameInfos();
__int32 __cdecl LoadNewStrings();