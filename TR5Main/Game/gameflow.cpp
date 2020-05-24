#include "gameflow.h"
#include "draw.h"
#include "savegame.h"

#include "input.h"
#include "global.h"
#include "level.h"

#include <string>

using namespace std;

SavegameInfo g_SavegameInfos[MAX_SAVEGAMES];
SaveGameHeader g_NewSavegameInfos[MAX_SAVEGAMES];
vector<string> g_NewStrings;

extern GameFlow* g_GameFlow;

int LoadSavegameInfos()
{
	char fileName[255];

	for (int i = 0; i < MAX_SAVEGAMES; i++)
	{
		g_NewSavegameInfos[i].Present = false;
	}

	// try to load the savegame
	for (int i = 0; i < MAX_SAVEGAMES; i++)
	{
		ZeroMemory(fileName, 255);
		sprintf(fileName, "savegame.%d", i);

		FILE* savegamePtr = fopen(fileName, "rb");
		if (savegamePtr == NULL)
			continue;
		fclose(savegamePtr);

		g_NewSavegameInfos[i].Present = true;
		SaveGame::LoadHeader(fileName, &g_NewSavegameInfos[i]);

		fclose(savegamePtr);
	}

	return 0;
}
