#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "ChunkWriter.h"
#include "LEB128.h"
#include "Streams.h"
#include "GameFlowScript.h"
#include "GameLogicScript.h"	

#define SAVEGAME_BUFFER_SIZE 1048576

struct STATS
{
	unsigned int Timer;
	unsigned int Distance;
	unsigned int AmmoUsed;
	unsigned int AmmoHits;
	unsigned short Kills;
	unsigned char Secrets;
	unsigned char HealthUsed;
};

struct SAVEGAME_INFO
{
	short Checksum;
	unsigned short VolumeCD;
	unsigned short VolumeFX;
	short ScreenX;
	short ScreenY;
	unsigned char ControlOption;
	bool VibrateOn;
	bool AutoTarget;
	STATS Level;
	STATS Game;
	short WeaponObject;
	short WeaponAnim;
	short WeaponFrame;
	short WeaponCurrent;
	short WeaponGoal;
	unsigned int CutSceneTriggered1;
	unsigned int CutSceneTriggered2;
	byte GameComplete;
	unsigned char LevelNumber;
	unsigned char CampaignSecrets[4];
	unsigned char TLCount;
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

extern SAVEGAME_INFO Savegame;

class SaveGame {
private:
	static FileStream* m_stream;
	static std::vector<LuaVariable> m_luaVariables;
	
public:
	static int LastSaveGame;

	static bool Load(char* fileName);
	static bool LoadHeader(char* fileName, SaveGameHeader* header);
	static bool Save(char* fileName);
};