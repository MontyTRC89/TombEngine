#include "framework.h"
#include "LoadSave.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Game/savegame.h"

int S_LoadSave(int loadSave)
{
	//if (loadSave == IN_LOAD)
	//	SaveGame::Load("savegame.save");
	//else
	//	SaveGame::Save("savegame.save");
	if (loadSave == IN_SAVE)
	{
		char fileName[255];
		ZeroMemory(fileName, 255);
		sprintf(fileName, "savegame.%d", 0);
		SaveGame::Save(fileName);
	}

	return 1;
}