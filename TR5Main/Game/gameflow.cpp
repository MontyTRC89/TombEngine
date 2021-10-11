#include "framework.h"
#include "gameflow.h"
#include "animation.h"
#include "savegame.h"
#include "input.h"

#include "level.h"
using std::string;
using std::vector;
SavegameInfo g_SavegameInfos[MAX_SAVEGAMES];
SaveGameHeader g_NewSavegameInfos[MAX_SAVEGAMES];
vector<string> g_NewStrings;

int LoadSavegameInfos()
{
	char fileName[255];

	for (int i = 0; i < MAX_SAVEGAMES; i++)
	{
		g_NewSavegameInfos[i].present = false;
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

		g_NewSavegameInfos[i].present = true;
		SaveGame::LoadHeader(fileName, &g_NewSavegameInfos[i]);

		fclose(savegamePtr);
	}

	return 0;
}
