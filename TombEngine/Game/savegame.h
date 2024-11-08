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
	int LevelHash;
	int Days;
	int Hours;
	int Minutes;
	int Seconds;
	int Level;
	int Timer;
	int Count;
	bool Present;
};

class SaveGame 
{
private:
	static std::string FullSaveDirectory;
	static int LastSaveGame;
	static std::map<int, std::vector<byte>> Hub;

	static std::string SaveGame::GetSavegameFilename(int slot);
	static bool IsSaveGameSlotValid(int slot);

	static const std::vector<byte> Build();
	static void Parse(const std::vector<byte>& buffer, bool hubMode);

public:
	static GameStats Statistics;
	static SaveGameHeader Infos[SAVEGAME_MAX];

	static void Init(const std::string& dir);
	static bool Load(int slot);
	static bool LoadHeader(int slot, SaveGameHeader* header);
	static void LoadHeaders();
	static bool Save(int slot);
	static void Delete(int slot);

	static bool DoesSaveGameExist(int slot, bool silent = false);
	static bool IsLoadGamePossible();

	static void SaveHub(int index);
	static void LoadHub(int index);
	static bool IsOnHub(int index);
	static void ResetHub();
};
