#pragma once
#include "Flow/ScriptInterfaceFlowHandler.h"
#include "Specific/IO/Streams.h"
#include <chrono>
#include <filesystem>

namespace TEN
{
	namespace Save
	{
		struct SaveGame;
	}
}

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
	std::filesystem::file_time_type Modified;
};

extern GameStats Statistics;
extern SaveGameHeader SavegameInfos[SAVEGAME_MAX];

class SaveGame 
{
private:
	static FileStream* m_stream;
	
public:
	static int LastSaveGame;

	static bool Load(int slot);
	static bool LoadHeader(int slot, SaveGameHeader* header);
	static bool Save(int slot);
	static void LoadLuaVarsOnly(int slot, bool gameVarsOnly = false);
};

void LoadSavegameInfos();
int GetMostRecentSave();
