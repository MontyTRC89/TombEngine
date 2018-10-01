#include "savegame.h"
#include "..\Global\global.h"

FileStream* SaveGame::m_stream;
ChunkReader* SaveGame::m_reader;
ChunkWriter* SaveGame::m_writer;

ChunkId* SaveGame::m_chunkHeader;
ChunkId* SaveGame::m_chunkGameStatus;
ChunkId* SaveGame::m_chunkItems;
ChunkId* SaveGame::m_chunkItem;
ChunkId* SaveGame::m_chunkLara;
ChunkId* SaveGame::m_chunkLuaVariables;
ChunkId* SaveGame::m_chunkLuaLocal;
ChunkId* SaveGame::m_chunkLuaGlobal;

void SaveGame::SaveHeader(__int32 param)
{
	m_stream->WriteString(g_GameFlow->GetString(g_GameFlow->GetLevel(CurrentLevel)->Name));
	LEB128::Write(m_stream, (Savegame.Game.Timer / 30) / 86400);
	LEB128::Write(m_stream, ((Savegame.Game.Timer / 30) % 86400) / 3600);
	LEB128::Write(m_stream, ((Savegame.Game.Timer / 30) / 60) % 60);
	LEB128::Write(m_stream, (Savegame.Game.Timer / 30) % 60);
	LEB128::Write(m_stream, CurrentLevel);
}

void SaveGame::SaveGameStatus(__int32 param)
{

}

void SaveGame::SaveItems(__int32 param)
{
	for (__int32 i = 0; i < NumItems; i++)
	{
		m_writer->WriteChunk(m_chunkItem, &SaveItem, i);
	}
}

void SaveGame::SaveItem(__int32 itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	LEB128::Write(m_stream, item->flags);
	LEB128::Write(m_stream, item->pos.xPos);
	LEB128::Write(m_stream, item->pos.yPos);
	LEB128::Write(m_stream, item->pos.zPos);
	LEB128::Write(m_stream, item->pos.xRot);
	LEB128::Write(m_stream, item->pos.yRot);
	LEB128::Write(m_stream, item->pos.zRot);
	LEB128::Write(m_stream, item->roomNumber);
	LEB128::Write(m_stream, item->itemFlags[0]);
	LEB128::Write(m_stream, item->itemFlags[1]);
	LEB128::Write(m_stream, item->itemFlags[2]);
	LEB128::Write(m_stream, item->itemFlags[3]);
	LEB128::Write(m_stream, item->triggerFlags);
	LEB128::Write(m_stream, item->timer);
	LEB128::Write(m_stream, item->speed);
	LEB128::Write(m_stream, item->fallspeed);
	LEB128::Write(m_stream, item->currentAnimState);
	LEB128::Write(m_stream, item->goalAnimState);
	LEB128::Write(m_stream, item->requiredAnimState);
	LEB128::Write(m_stream, item->animNumber);
	LEB128::Write(m_stream, item->frameNumber);
	LEB128::Write(m_stream, item->objectNumber);
	LEB128::Write(m_stream, item->hitPoints);
}

void SaveGame::SaveLara(__int32 param)
{
	LARA_INFO lara;
	memcpy(&lara, &Lara, sizeof(Lara));

	for (__int32 i = 0; i < 15; i++)
		lara.meshPtrs[i] = (__int16*)(lara.meshPtrs[i] - MeshBase);

	lara.leftArm.frameBase = (__int16*)(lara.leftArm.frameBase - Objects[ID_PISTOLS_ANIM].frameBase);
	lara.rightArm.frameBase = (__int16*)(lara.rightArm.frameBase - Objects[ID_PISTOLS_ANIM].frameBase);

	m_stream->Write(reinterpret_cast<char*>(&lara), sizeof(Lara));
}

bool SaveGame::Save(char* fileName)
{
	m_chunkHeader = ChunkId::FromString("TR5MSgHeader");
	m_chunkGameStatus = ChunkId::FromString("TR5MSgGameStatus");
	m_chunkItems = ChunkId::FromString("TR5MSgItems");
	m_chunkItem = ChunkId::FromString("TR5MSgItem");
	m_chunkLara = ChunkId::FromString("TR5MSgLara");
	m_chunkLuaVariables = ChunkId::FromString("TR5MSgLuaVars");
	m_chunkLuaLocal = ChunkId::FromString("TR5MSgLuaL");
	m_chunkLuaGlobal = ChunkId::FromString("TR5MSgLuaG");

	m_stream = new FileStream(fileName);
	m_writer = new ChunkWriter(0x4D355254, m_stream);

	m_writer->WriteChunk(m_chunkHeader, &SaveHeader, 0);
	m_writer->WriteChunk(m_chunkLara, &SaveLara, 0);
	m_writer->WriteChunkWithChildren(m_chunkItems, &SaveItems, 0);

	m_stream->Close();

	delete m_writer;
	delete m_stream;

	delete m_chunkHeader;
	delete m_chunkGameStatus;
	delete m_chunkItems;
	delete m_chunkItem;
	delete m_chunkLara;
	delete m_chunkLuaVariables;
	delete m_chunkLuaLocal;
	delete m_chunkLuaGlobal;

	return true;
}