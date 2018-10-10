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

class SaveGame {
private:
	static FileStream* m_stream;
	static ChunkReader* m_reader;
	static ChunkWriter* m_writer;
	static vector<LuaVariable> m_luaVariables;

	static ChunkId* m_chunkHeader;
	static ChunkId* m_chunkGameStatus;
	static ChunkId* m_chunkItems;
	static ChunkId* m_chunkItem;
	static ChunkId* m_chunkLara;
	static ChunkId* m_chunkLuaVariable;

	static void saveHeader(__int32 param);
	static void saveGameStatus(__int32 param);
	static void saveLara(__int32 param);
	static void saveItem(__int32 param);
	static void saveItems(__int32 param);
	static void saveVariables(__int32 param);
	static void saveVariable(__int32 param);

	static bool readHeader();
	static bool readGameStatus();
	static bool readLara();
	static bool readItem();
	static bool readVariable();
	static bool readSavegameChunks(ChunkId* chunkId, __int32 maxSize);

public:
	static void Start();
	static void End();

	static bool Load(char* fileName);
	static bool Save(char* fileName); 
};