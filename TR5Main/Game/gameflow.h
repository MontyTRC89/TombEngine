#pragma once

#include <vector>
#include <string>
#include "savegame.h"

using namespace std;

typedef struct SavegameInfo {
	bool present;
	char levelName[75];
	int saveNumber;
	short days;
	short hours;
	short minutes;
	short seconds;
	char fileName[255];
};

#define MAX_SAVEGAMES 16

extern SavegameInfo g_SavegameInfos[MAX_SAVEGAMES];
extern vector<string> g_NewStrings;
extern SaveGameHeader g_NewSavegameInfos[MAX_SAVEGAMES];

int LoadSavegameInfos();