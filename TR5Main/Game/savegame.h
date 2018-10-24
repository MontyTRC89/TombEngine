#pragma once

#include "..\Global\global.h"
#include "..\Specific\IO\ChunkId.h"
#include "..\Specific\IO\ChunkReader.h"
#include "..\Specific\IO\ChunkWriter.h"
#include "..\Specific\IO\LEB128.h"
#include "..\Specific\IO\Streams.h"
#include "..\Scripting\GameFlowScript.h"
#include "..\Scripting\GameLogicScript.h"

#define RestoreGame ((__int32 (__cdecl*)()) 0x00472060)	
#define ReadSavegame ((__int32 (__cdecl*)(__int32)) 0x004A8E10)	
#define CreateSavegame ((void (__cdecl*)()) 0x00470FA0)	
#define WriteSavegame ((__int32 (__cdecl*)(__int32)) 0x004A8BC0)	

#define SAVEGAME_BUFFER_SIZE 1048576

extern GameFlow* g_GameFlow;
extern GameScript* g_GameScript;

typedef struct SaveGameHeader
{
	string LevelName;
	__int32 Days;
	__int32 Hours;
	__int32 Minutes;
	__int32 Seconds;
	__int32 Level;
	__int32 Timer;
	__int32 Count;
	bool Present;
};

class SaveGame {
private:
	static FileStream* m_stream;
	static ChunkReader* m_reader;
	static ChunkWriter* m_writer;
	static vector<LuaVariable> m_luaVariables;

	static ChunkId* m_chunkGameStatus;
	static ChunkId* m_chunkItems;
	static ChunkId* m_chunkItem;
	static ChunkId* m_chunkLara;
	static ChunkId* m_chunkLuaVariable;
	static ChunkId* m_chunkStaticFlags;
	static ChunkId* m_chunkLaraVehicle;
	static ChunkId* m_chunkFlybySequence;
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

	static void saveGameStatus(__int32 arg1, __int32 arg2);
	static void saveLara(__int32 arg1, __int32 arg2);
	static void saveItem(__int32 arg1, __int32 arg2);
	static void saveItems();
	static void saveVariables();
	static void saveVariable(__int32 arg1, __int32 arg2);
	static void saveStaticFlag(__int32 arg1, __int32 arg2);
	static void saveCamera(__int32 arg1, __int32 arg2);
	static void saveFlybySequence(__int32 arg1, __int32 arg2);
	static void saveFlybyFlags(__int32 arg1, __int32 arg2);
	static void saveFlipMap(__int32 arg1, __int32 arg2);
	static void saveFlipStats(__int32 arg1, __int32 arg2);
	static void saveCdFlags(__int32 arg1, __int32 arg2);
	static void saveStatistics(__int32 arg1, __int32 arg2);
	static void saveItemFlags(__int32 arg1, __int32 arg2);
	static void saveItemHitPoints(__int32 arg1, __int32 arg2);
	static void saveItemPosition(__int32 arg1, __int32 arg2);
	static void saveItemMesh(__int32 arg1, __int32 arg2);
	static void saveItemAnims(__int32 arg1, __int32 arg2);
	static void saveItemIntelligentData(__int32 arg1, __int32 arg2);

	static bool readGameStatus();
	static bool readLara();
	static bool readItem();
	static bool readStatistics();
	static bool readVariable();
	static bool readSavegameChunks(ChunkId* chunkId, __int32 maxSize, __int32 arg);
	static bool readLaraChunks(ChunkId* chunkId, __int32 maxSize, __int32 arg);
	static bool readGameStatusChunks(ChunkId* chunkId, __int32 maxSize, __int32 arg);
	static bool readItemChunks(ChunkId* chunkId, __int32 maxSize, __int32 itemNumber);
	
public:
	static __int32 LastSaveGame;

	static void Start();
	static void End();

	static bool Load(char* fileName);
	static bool LoadHeader(char* fileName, SaveGameHeader* header);
	
	static bool Save(char* fileName);
};