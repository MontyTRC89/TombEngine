#include "gameflow.h"
#include "draw.h"
#include "savegame.h"

#include "..\Specific\input.h"
#include "..\Global\global.h"

#include <string>

using namespace std;

SavegameInfo g_SavegameInfos[MAX_SAVEGAMES];
SaveGameHeader g_NewSavegameInfos[MAX_SAVEGAMES];
vector<string> g_NewStrings;

extern GameFlow* g_GameFlow;

__int32 __cdecl LoadSavegameInfos()
{
	char fileName[255];

	for (__int32 i = 0; i < MAX_SAVEGAMES; i++)
	{
		g_NewSavegameInfos[i].Present = false;
	}

	// try to load the savegame
	for (__int32 i = 0; i < MAX_SAVEGAMES; i++)
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

__int32 __cdecl LoadNewStrings()
{
	FILE* filePtr = fopen("english.txt", "r");
	if (filePtr == NULL) return false;

	char buffer[1000];
	ZeroMemory(buffer, 1000);

	__int32 stringLength = 0;
	char c = getc(filePtr);

	while (c != EOF)
	{
		if (c == '\r')
		{
			c = getc(filePtr);
			continue;
		}

		if (c == '\n')
		{
			buffer[stringLength] = 0;
			g_NewStrings.push_back(string(buffer));
			ZeroMemory(buffer, 1000);
			c = getc(filePtr);
			stringLength = 0;
			continue;
		}

		buffer[stringLength] = c;
		c = getc(filePtr);
		stringLength++;
	}

	fclose(filePtr);

	return true;
}
