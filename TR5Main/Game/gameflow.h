#pragma once
#include <vector>
#include <string>

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

#define DoGameflow ((__int32 (__cdecl*)(void)) 0x004A8570)
#define LoadGameflow ((__int32 (__cdecl*)(void)) 0x00434800)

__int32 __cdecl DoPauseMenu();
__int32 __cdecl DoStatisticsMenu();
__int32 __cdecl DoSettingsMenu();
__int32 __cdecl DoLoadGameMenu();
__int32 __cdecl DoSaveGameMenu();
__int32 __cdecl LoadSavegameInfos();
__int32 __cdecl LoadNewStrings();
__int32 __cdecl DoPassportLoadGameMenu();