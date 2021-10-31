#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "ChunkWriter.h"
#include "LEB128.h"
#include "Streams.h"
#include "GameFlowScript.h"
#include "GameLogicScript.h"	

__declspec( selectany ) extern const std::string SAVEGAME_PATH = "Save//";
constexpr auto SAVEGAME_MAX = 16;

struct Stats
{
	unsigned int Timer;
	unsigned int Distance;
	unsigned int AmmoUsed;
	unsigned int AmmoHits;
	unsigned short Kills;
	unsigned char Secrets;
	unsigned char HealthUsed;
};

struct GameStats
{
	Stats Game;
	Stats Level;
};

struct SavegameInfo
{
	bool present;
	char levelName[75];
	int saveNumber;
	short days;
	short hours;
	short minutes;
	short seconds;
	char fileName[255];
};

struct SaveGameHeader
{
	std::string levelName;
	int days;
	int hours;
	int minutes;
	int seconds;
	int level;
	int timer;
	int count;
	bool present;
};

extern SavegameInfo g_SavegameInfos[SAVEGAME_MAX];
extern std::vector<std::string> g_NewStrings;
extern SaveGameHeader g_NewSavegameInfos[SAVEGAME_MAX];
extern GameStats Statistics;

class SaveGame 
{
private:
	static FileStream* m_stream;
	static std::vector<LuaVariable> m_luaVariables;
	
public:
	static int LastSaveGame;

	static bool Load(int slot);
	static bool LoadHeader(int slot, SaveGameHeader* header);
	static bool Save(int slot);
};

void LoadSavegameInfos();