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

__int32 __cdecl DoPauseMenu()
{
	// Dump the screen
	g_Renderer->DumpGameScene();

	__int32 choice = 0;
	g_Renderer->DrawPauseMenu(choice, false);

	while (true)
	{
		g_Renderer->DrawPauseMenu(choice, false);

		SetDebounce = 1;
		S_UpdateInput();
		SetDebounce = 0;

		// Process input
		if (DbInput & 1 && choice > 0)
		{
			choice--;
			continue;
		}
		else if (DbInput & 2 && choice < 2)
		{
			choice++;
			continue;
		}
		else if (DbInput & 0x200000)
		{
			break;
		}
		else if (DbInput & 0x100000)
		{
			if (choice == 0)
				DoStatisticsMenu();
		}
	}

	return 0;
}

__int32 __cdecl DoStatisticsMenu()
{
	g_Renderer->DrawStatisticsMenu();

	while (true)
	{
		g_Renderer->DrawStatisticsMenu();

		SetDebounce = 1;
		S_UpdateInput();
		SetDebounce = 0;

		// Process input
		if (DbInput & 0x200000)
		{
			return 0;
		}
	}

	return 0;
}

__int32 __cdecl DoSettingsMenu()
{
	return 0;
}

__int32 __cdecl DoLoadGameMenu()
{
	__int32 choice = 0;
	g_Renderer->DrawLoadGameMenu(choice, false);

	while (true)
	{
		SetDebounce = 1;
		S_UpdateInput();
		SetDebounce = 0;

		// Process input
		if (DbInput & 1 && choice > 0)
		{
			choice--;
			continue;
		}
		else if (DbInput & 2 && choice < MAX_SAVEGAMES - 1)
		{
			choice++;
			continue;
		}
		else if (DbInput & 0x200000)
		{
			break;
		}
		else if (DbInput & 0x100000)
		{
			//if (choice == 0)
			//	DoStatisticsMenu();
		}

		g_Renderer->DrawLoadGameMenu(choice, false);
		g_Renderer->SyncRenderer();
	}

	return 0;
}

__int32 __cdecl DoSaveGameMenu()
{
	return 0;
}

__int32 __cdecl LoadSavegameInfos()
{
	char fileName[255];

	// clean the array
	/*for (__int32 i = 0; i < MAX_SAVEGAMES; i++)
	{
		g_SavegameInfos[i].present = false;
	}

	// try to load the savegame
	for (__int32 i = 0; i < MAX_SAVEGAMES; i++)
	{
		ZeroMemory(fileName, 255);
		sprintf(fileName, "savegame.%d", i);

		FILE* savegamePtr = fopen(fileName, "rb");
		if (savegamePtr == NULL)
			continue;

		g_SavegameInfos[i].present = true;
		strcpy_s(g_SavegameInfos[i].fileName, 255, fileName);

		fread(&g_SavegameInfos[i].levelName, 1, 75, savegamePtr);
		fread(&g_SavegameInfos[i].saveNumber, 4, 1, savegamePtr);
		fread(&g_SavegameInfos[i].days, 2, 1, savegamePtr);
		fread(&g_SavegameInfos[i].hours, 2, 1, savegamePtr);
		fread(&g_SavegameInfos[i].minutes, 2, 1, savegamePtr);
		fread(&g_SavegameInfos[i].seconds, 2, 1, savegamePtr);

		fclose(savegamePtr);
	}*/

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

__int32 __cdecl DoPassportLoadGameMenu()
{
	__int32 choice = 0;
	g_Renderer->DrawLoadGameMenu(choice, false);

	while (true)
	{
		SetDebounce = 1;
		S_UpdateInput();
		SetDebounce = 0;

		// Process input
		if (DbInput & 1 && choice > 0)
		{
			choice--;
			continue;
		}
		else if (DbInput & 2 && choice < MAX_SAVEGAMES - 1)
		{
			choice++;
			continue;
		}
		else if (DbInput & 4 || DbInput & 8)
		{
			break;
		}
		else if (DbInput & 0x100000)
		{
			//if (choice == 0)
			//	DoStatisticsMenu();
		}

		g_Renderer->DrawLoadGameMenu(choice, false);
		g_Renderer->SyncRenderer();
	}

	return 0;
}