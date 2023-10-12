#pragma once
#include "Scripting/Include/Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/IO/ChunkId.h"
#include "Specific/IO/ChunkReader.h"
#include "Specific/IO/ChunkWriter.h"
#include "Specific/IO/LEB128.h"
#include "Specific/IO/Streams.h"

constexpr auto SAVEGAME_MAX = 16;

struct Stats
{
	unsigned int Timer;
	unsigned int Distance;
	unsigned int AmmoHits;
	unsigned int AmmoUsed;
	unsigned int HealthUsed;
	unsigned int Kills;
	unsigned int Secrets;
};

struct GameStats
{
	Stats Game;
	Stats Level;
};

struct SaveGameHeader
{
	std::string LevelName;
	int Days;
	int Hours;
	int Minutes;
	int Seconds;
	int Level;
	int Timer;
	int Count;
	bool Present;
};

extern GameStats Statistics;
extern SaveGameHeader SavegameInfos[SAVEGAME_MAX];

class SaveGame 
{
private:
	static FileStream* StreamPtr;
	static std::string FullSaveDirectory;

	static std::string SaveGame::GetSavegameFilename(int slot);
	static bool IsSaveGameSlotValid(int slot);
	
public:
	static int LastSaveGame;

	static void Init(const std::string& dir);
	static bool Load(int slot);
	static bool LoadHeader(int slot, SaveGameHeader* header);
	static bool Save(int slot);
	static void LoadSavegameInfos();
	static void Delete(int slot);

	static bool DoesSaveGameExist(int slot, bool silent = false);
};
