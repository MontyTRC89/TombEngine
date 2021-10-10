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

extern SAVEGAME_INFO Savegame;

class SaveGame {
private:
	static FileStream* m_stream;
	static ChunkReader* m_reader;
	static ChunkWriter* m_writer;
	static std::vector<LuaVariable> m_luaVariables;

	static std::unique_ptr<ChunkId> m_chunkGameStatus;
	static std::unique_ptr<ChunkId> m_chunkItems;
	static std::unique_ptr<ChunkId> m_chunkItem;
	static std::unique_ptr<ChunkId> m_chunkLara;
	static std::unique_ptr<ChunkId> m_chunkLuaVariable;
	static std::unique_ptr<ChunkId> m_chunkStaticFlags;
	static std::unique_ptr<ChunkId> m_chunkVehicle;
	static std::unique_ptr<ChunkId> m_chunkSequenceSwitch;
	static std::unique_ptr<ChunkId> m_chunkFlybyFlags;
	static std::unique_ptr<ChunkId> m_chunkCdFlags;
	static std::unique_ptr<ChunkId> m_chunkCamera;
	static std::unique_ptr<ChunkId> m_chunkFlipStats;
	static std::unique_ptr<ChunkId> m_chunkFlipMap;
	static std::unique_ptr<ChunkId> m_chunkItemDummy;
	static std::unique_ptr<ChunkId> m_chunkStatistics;
	static std::unique_ptr<ChunkId> m_chunkItemAnims;
	static std::unique_ptr<ChunkId> m_chunkItemMeshes;
	static std::unique_ptr<ChunkId> m_chunkItemFlags;
	static std::unique_ptr<ChunkId> m_chunkItemHitPoints;
	static std::unique_ptr<ChunkId> m_chunkItemPosition;
	static std::unique_ptr<ChunkId> m_chunkItemIntelligentData;
	static std::unique_ptr<ChunkId> m_chunkSpecialItemBurningTorch;
	static std::unique_ptr<ChunkId> m_chunkSpecialItemChaff;
	static std::unique_ptr<ChunkId> m_chunkSpecialItemTorpedo;
	static std::unique_ptr<ChunkId> m_chunkSpecialItemCrossbowBolt;
	static std::unique_ptr<ChunkId> m_chunkSpecialItemFlare;
	static std::unique_ptr<ChunkId> m_chunkItemQuadInfo;
	static std::unique_ptr<ChunkId> m_chunkRats;
	static std::unique_ptr<ChunkId> m_chunkSpiders;
	static std::unique_ptr<ChunkId> m_chunkBats;
	static std::unique_ptr<ChunkId> m_chunkLaraExtraInfo;
	static std::unique_ptr<ChunkId> m_chunkWeaponInfo;
	static std::unique_ptr<ChunkId> m_chunkPuzzle;
	static std::unique_ptr<ChunkId> m_chunkKey;
	static std::unique_ptr<ChunkId> m_chunkPickup;
	static std::unique_ptr<ChunkId> m_chunkExamine;
	static std::unique_ptr<ChunkId> m_chunkPuzzleCombo;
	static std::unique_ptr<ChunkId> m_chunkKeyCombo;
	static std::unique_ptr<ChunkId> m_chunkPickupCombo;
	static std::unique_ptr<ChunkId> m_chunkExamineCombo;
	static std::unique_ptr<ChunkId> m_chunkWeaponItem;

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