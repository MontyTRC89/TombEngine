#pragma once
#include "ChunkId.h"
#include "ChunkReader.h"
#include "ChunkWriter.h"
#include "LEB128.h"
#include "Streams.h"
#include "GameFlowScript.h"
#include "GameLogicScript.h"	

#define SAVEGAME_BUFFER_SIZE 1048576

typedef struct STATS
{
	unsigned int Timer;
	unsigned int Distance;
	unsigned int AmmoUsed;
	unsigned int AmmoHits;
	unsigned short Kills;
	unsigned char Secrets;
	unsigned char HealthUsed;
};

typedef struct SAVEGAME_INFO
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

typedef struct SaveGameHeader
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

extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;
extern SAVEGAME_INFO Savegame;

class SaveGame {
private:
	static FileStream* m_stream;
	static ChunkReader* m_reader;
	static ChunkWriter* m_writer;
	static std::vector<LuaVariable> m_luaVariables;

	static ChunkId* m_chunkGameStatus;
	static ChunkId* m_chunkItems;
	static ChunkId* m_chunkItem;
	static ChunkId* m_chunkLara;
	static ChunkId* m_chunkLuaVariable;
	static ChunkId* m_chunkStaticFlags;
	static ChunkId* m_chunkVehicle;
	static ChunkId* m_chunkSequenceSwitch;
	static ChunkId* m_chunkFlybyFlags;
	static ChunkId* m_chunkCdFlags;
	static ChunkId* m_chunkCamera;
	static ChunkId* m_chunkFlipStats;
	static ChunkId* m_chunkFlipMap;
	static ChunkId* m_chunkItemDummy;
	static ChunkId* m_chunkStatistics;
	static ChunkId* m_chunkItemAnims;
	static ChunkId* m_chunkItemMeshes;
	static ChunkId* m_chunkItemFlags;
	static ChunkId* m_chunkItemHitPoints;
	static ChunkId* m_chunkItemPosition;
	static ChunkId* m_chunkItemIntelligentData;
	static ChunkId* m_chunkSpecialItemBurningTorch;
	static ChunkId* m_chunkSpecialItemChaff;
	static ChunkId* m_chunkSpecialItemTorpedo;
	static ChunkId* m_chunkSpecialItemCrossbowBolt;
	static ChunkId* m_chunkSpecialItemFlare;
	static ChunkId* m_chunkItemQuadInfo;
	static ChunkId* m_chunkRats;
	static ChunkId* m_chunkSpiders;
	static ChunkId* m_chunkBats;
	static ChunkId* m_chunkLaraExtraInfo;
	static ChunkId* m_chunkWeaponInfo;
	static ChunkId* m_chunkPuzzle;
	static ChunkId* m_chunkKey;
	static ChunkId* m_chunkPickup;
	static ChunkId* m_chunkExamine;
	static ChunkId* m_chunkPuzzleCombo;
	static ChunkId* m_chunkKeyCombo;
	static ChunkId* m_chunkPickupCombo;
	static ChunkId* m_chunkExamineCombo;
	static ChunkId* m_chunkWeaponItem;

	static void saveGameStatus(int arg1, int arg2);
	static void saveLara(int arg1, int arg2);
	static void saveItem(int arg1, int arg2);
	static void saveBurningTorch(int arg1, int arg2);
	static void saveChaff(int arg1, int arg2);
	static void saveTorpedo(int arg1, int arg2);
	static void saveCrossbowBolt(int arg1, int arg2);
	static void saveFlare(int arg1, int arg2);
	static void saveItems();
	static void saveVariables();
	static void saveVariable(int arg1, int arg2);
	static void saveStaticFlag(int arg1, int arg2);
	static void saveCamera(int arg1, int arg2);
	static void saveSequenceSwitch(int arg1, int arg2);
	static void saveFlybyFlags(int arg1, int arg2);
	static void saveFlipMap(int arg1, int arg2);
	static void saveFlipStats(int arg1, int arg2);
	static void saveCdFlags(int arg1, int arg2);
	static void saveStatistics(int arg1, int arg2);
	static void saveItemFlags(int arg1, int arg2);
	static void saveItemHitPoints(int arg1, int arg2);
	static void saveItemPosition(int arg1, int arg2);
	static void saveItemMesh(int arg1, int arg2);
	static void saveItemAnims(int arg1, int arg2);
	static void saveItemIntelligentData(int arg1, int arg2);
	static void saveItemQuadInfo(int arg1, int arg2);
	static void saveRats(int arg1, int arg2);
	static void saveBats(int arg1, int arg2);
	static void saveSpiders(int arg1, int arg2);
	static void saveLaraExtraInfo(int arg1, int arg2);
	static void savePuzzle(int arg1, int arg2);
	static void saveWeaponInfo(int arg1, int arg2);
	static void saveWeaponItem(int arg1, int arg2);

	static bool readGameStatus();
	static bool readLara();
	static bool readItem();
	static bool readBurningTorch();
	static bool readChaff();
	static bool readCrossbowBolt();
	static bool readFlare();
	static bool readTorpedo();
	static bool readBats();
	static bool readRats();
	static bool readSpiders();
	static bool readStatistics();
	static bool readVariable();
	static bool readSavegameChunks(ChunkId* chunkId, int maxSize, int arg);
	static bool readLaraChunks(ChunkId* chunkId, int maxSize, int arg);
	static bool readGameStatusChunks(ChunkId* chunkId, int maxSize, int arg);
	static bool readItemChunks(ChunkId* chunkId, int maxSize, int itemNumber);
	
public:
	static int LastSaveGame;

	static void Start();
	static void End();
	static bool Load(char* fileName);
	static bool LoadHeader(char* fileName, SaveGameHeader* header);
	static bool Save(char* fileName);
};