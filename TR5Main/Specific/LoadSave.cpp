#include "framework.h"
#include "LoadSave.h"
#include "Sound/sound.h"
#include "Specific/input.h"
#include "Game/savegame.h"

int S_LoadSave(int loadSave)
{
	if (loadSave == IN_LOAD)
		SaveGame::Load("savegame.save");
	else
		SaveGame::Save("savegame.save");

	return 1;
}