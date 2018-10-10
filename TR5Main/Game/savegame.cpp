#include "savegame.h"
#include "..\Global\global.h"
#include "..\Game\lara.h"
#include "..\Game\items.h"
#include "..\Game\laramisc.h"

FileStream* SaveGame::m_stream;
ChunkReader* SaveGame::m_reader;
ChunkWriter* SaveGame::m_writer;
vector<LuaVariable> SaveGame::m_luaVariables;

ChunkId* SaveGame::m_chunkHeader;
ChunkId* SaveGame::m_chunkGameStatus;
ChunkId* SaveGame::m_chunkItems;
ChunkId* SaveGame::m_chunkItem;
ChunkId* SaveGame::m_chunkLara;
ChunkId* SaveGame::m_chunkLuaVariable;

void SaveGame::saveHeader(__int32 param)
{
	m_stream->WriteString(g_GameFlow->GetString(g_GameFlow->GetLevel(CurrentLevel)->Name));
	LEB128::Write(m_stream, (Savegame.Game.Timer / 30) / 86400);
	LEB128::Write(m_stream, ((Savegame.Game.Timer / 30) % 86400) / 3600);
	LEB128::Write(m_stream, ((Savegame.Game.Timer / 30) / 60) % 60);
	LEB128::Write(m_stream, (Savegame.Game.Timer / 30) % 60);
	LEB128::Write(m_stream, CurrentLevel);
	LEB128::Write(m_stream, GameTimer);
}

void SaveGame::saveItems(__int32 param)
{
	for (__int32 i = 0; i < NumItems; i++)
	{
		m_writer->WriteChunk(m_chunkItem, &saveItem, i);
	}
}

void SaveGame::saveItem(__int32 itemNumber)
{
	ITEM_INFO* item = &Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	LEB128::Write(m_stream, itemNumber);
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

void SaveGame::saveGameStatus(__int32 param)
{
	LEB128::Write(m_stream, FlipStatus);
	for (__int32 i = 0; i < 255; i++)
		LEB128::Write(m_stream, FlipStats[i]);
	for (__int32 i = 0; i < 255; i++)
		LEB128::Write(m_stream, FlipMap[i]);

	LEB128::Write(m_stream, GlobalLastInventoryItem);
	LEB128::Write(m_stream, FlipEffect);
	LEB128::Write(m_stream, FlipTimer);
	for (__int32 i = 0; i < 136; i++)
		LEB128::Write(m_stream, CdFlags[i]);
	LEB128::Write(m_stream, CurrentAtmosphere);

	// Shatters
	if (NumberRooms > 0)
	{
		for (__int32 i = 0; i < NumberRooms; i++)
			for (__int32 j = 0; j < Rooms[i].numMeshes; j++)
				LEB128::Write(m_stream, Rooms[i].mesh[j].Flags & 1);
	}

	// Flyby cameras
	LEB128::Write(m_stream, CurrentSequence);
	for (__int32 i = 0; i < 6; i++)
		LEB128::Write(m_stream, SequenceUsed[i]);
	for (__int32 i = 0; i < NumberSpotcams; i++)
		LEB128::Write(m_stream, SpotCam[i].flags);

	// Cameras
	for (__int32 i = 0; i < NumberCameras; i++)
		LEB128::Write(m_stream, Camera.fixed[i].flags);
}

void SaveGame::saveLara(__int32 param)
{
	// LARA_INFO struct dumped to savegame
	LARA_INFO lara;
	memcpy(&lara, &Lara, sizeof(Lara));

	for (__int32 i = 0; i < 15; i++)
		lara.meshPtrs[i] = (__int16*)(lara.meshPtrs[i] - MeshBase);

	lara.leftArm.frameBase = (__int16*)(lara.leftArm.frameBase - Objects[ID_PISTOLS_ANIM].frameBase);
	lara.rightArm.frameBase = (__int16*)(lara.rightArm.frameBase - Objects[ID_PISTOLS_ANIM].frameBase);

	m_stream->Write(reinterpret_cast<char*>(&lara), sizeof(Lara));
	
	// Lara weapon data
	ITEM_INFO* weaponItem = &Items[Lara.weaponItem];

	LEB128::Write(m_stream, weaponItem->objectNumber);
	LEB128::Write(m_stream, weaponItem->animNumber);
	LEB128::Write(m_stream, weaponItem->frameNumber);
	LEB128::Write(m_stream, weaponItem->currentAnimState);
	LEB128::Write(m_stream, weaponItem->goalAnimState);
	LEB128::Write(m_stream, weaponItem->requiredAnimState);

	// Lara extra data
	LEB128::Write(m_stream, LaraVehicle);
}

void SaveGame::saveVariables(__int32 param)
{
	m_luaVariables.clear();
	g_GameScript->GetVariables(&m_luaVariables);
	for (__int32 i = 0; i < m_luaVariables.size(); i++)
		m_writer->WriteChunk(m_chunkLuaVariable, &saveVariable, i);
}

void SaveGame::saveVariable(__int32 param)
{
	LuaVariable variable = m_luaVariables[param];
	
	LEB128::Write(m_stream, variable.IsGlobal == 1);
	LEB128::Write(m_stream, variable.Type);
	m_stream->WriteString((char*)variable.Name.c_str());

	if (variable.Type == LUA_VARIABLE_TYPE_BOOL)
		m_stream->WriteBool(variable.BoolValue);
	else if (variable.Type == LUA_VARIABLE_TYPE_INT)
		m_stream->WriteInt32(variable.IntValue);
	else if (variable.Type == LUA_VARIABLE_TYPE_STRING)
		m_stream->WriteString((char*)variable.StringValue.c_str());
	else if (variable.Type == LUA_VARIABLE_TYPE_FLOAT)
		m_stream->WriteFloat(variable.FloatValue);
}

void SaveGame::Start()
{
	m_chunkHeader = ChunkId::FromString("TR5MSgHeader");
	m_chunkGameStatus = ChunkId::FromString("TR5MSgGameStatus");
	m_chunkItems = ChunkId::FromString("TR5MSgItems");
	m_chunkItem = ChunkId::FromString("TR5MSgItem");
	m_chunkLara = ChunkId::FromString("TR5MSgLara");
	m_chunkLuaVariable = ChunkId::FromString("TR5MSgLuaVar");
}

void SaveGame::End()
{
	delete m_chunkHeader;
	delete m_chunkGameStatus;
	delete m_chunkItems;
	delete m_chunkItem;
	delete m_chunkLara;
	delete m_chunkLuaVariable;
}

bool SaveGame::Save(char* fileName)
{
	m_stream = new FileStream(fileName);
	m_writer = new ChunkWriter(0x4D355254, m_stream);

	m_writer->WriteChunk(m_chunkHeader, &saveHeader, 0);
	m_writer->WriteChunk(m_chunkGameStatus, &saveGameStatus, 0);
	m_writer->WriteChunk(m_chunkLara, &saveLara, 0);
	saveItems(0);
	saveVariables(0);

	// EOF
	m_stream->WriteInt32(0);

	m_stream->Close();

	delete m_writer;
	delete m_stream;

	return true;
}

bool SaveGame::readHeader()
{
	char* levelName;
	m_stream->ReadString(&levelName);

	// Skip timer
	LEB128::ReadInt32(m_stream);
	LEB128::ReadInt32(m_stream);
	LEB128::ReadInt32(m_stream);
	LEB128::ReadInt32(m_stream);

	CurrentLevel = LEB128::ReadByte(m_stream);
	GameTimer = LEB128::ReadInt32(m_stream);
}

bool SaveGame::readGameStatus()
{

}

bool SaveGame::readLara()
{
	// Read dumped LARA_INFO struct
	char* buffer = (char*)malloc(sizeof(LARA_INFO));
	m_stream->Read(buffer, sizeof(LARA_INFO));
	LARA_INFO* lara = reinterpret_cast<LARA_INFO*>(buffer);

	for (__int32 i = 0; i < 15; i++)
		lara->meshPtrs[i] = ADD_PTR(lara->meshPtrs[i], __int16, MeshBase);

	lara->leftArm.frameBase = ADD_PTR(lara->leftArm.frameBase, __int16, Objects[ID_PISTOLS_ANIM].frameBase);
	lara->rightArm.frameBase = ADD_PTR(lara->rightArm.frameBase, __int16, Objects[ID_PISTOLS_ANIM].frameBase);
	
	lara->target = NULL;
	lara->spazEffect = NULL;
	lara->generalPtr = ADD_PTR(lara->generalPtr, char, MallocBuffer);

	// Is Lara burning?
	if (lara->burn)
	{
		lara->burn = false;
		bool smokeFlag = false;
		if (lara->burnSmoke)
		{
			lara->burnSmoke = false;
			smokeFlag = true;
		}
		
		LaraBurn();
		if (smokeFlag)
			lara->burnSmoke = true;
	}

	// Lara weapon data
	if (lara->weaponItem)
	{
		__int16 weaponItemNum = CreateItem();
		lara->weaponItem = weaponItemNum;

		ITEM_INFO* weaponItem = &Items[lara->weaponItem];

		weaponItem->objectNumber = LEB128::ReadInt16(m_stream);
		weaponItem->animNumber = LEB128::ReadInt16(m_stream);
		weaponItem->frameNumber = LEB128::ReadInt16(m_stream);
		weaponItem->currentAnimState = LEB128::ReadInt16(m_stream);
		weaponItem->goalAnimState = LEB128::ReadInt16(m_stream);
		weaponItem->requiredAnimState = LEB128::ReadInt16(m_stream);
		weaponItem->roomNumber = 255;
	}
	
	// Lara extra data
	LaraVehicle = LEB128::ReadInt16(m_stream);

	// Copy temp struct to final struct
	memcpy(&Lara, lara, sizeof(LARA_INFO));
	free(buffer);
}

bool SaveGame::readItem()
{
	__int16 itemNumber = LEB128::ReadInt16(m_stream);

	ITEM_INFO* item = &Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	item->flags = LEB128::ReadInt16(m_stream);
	item->pos.xPos = LEB128::ReadInt32(m_stream);
	item->pos.yPos = LEB128::ReadInt32(m_stream);
	item->pos.zPos = LEB128::ReadInt32(m_stream);
	item->pos.xRot = LEB128::ReadInt16(m_stream);
	item->pos.yRot = LEB128::ReadInt16(m_stream);
	item->pos.zRot = LEB128::ReadInt16(m_stream);
	item->roomNumber = LEB128::ReadInt16(m_stream);
	item->itemFlags[0] = LEB128::ReadInt16(m_stream);
	item->itemFlags[1] = LEB128::ReadInt16(m_stream);
	item->itemFlags[2] = LEB128::ReadInt16(m_stream);
	item->itemFlags[3] = LEB128::ReadInt16(m_stream);
	item->triggerFlags = LEB128::ReadInt16(m_stream);
	item->timer = LEB128::ReadInt16(m_stream);
	item->speed = LEB128::ReadInt16(m_stream);
	item->fallspeed = LEB128::ReadInt16(m_stream);
	item->currentAnimState = LEB128::ReadInt16(m_stream);
	item->goalAnimState = LEB128::ReadInt16(m_stream);
	item->requiredAnimState = LEB128::ReadInt16(m_stream);
	item->animNumber = LEB128::ReadInt16(m_stream);
	item->frameNumber = LEB128::ReadInt16(m_stream);
	item->objectNumber = LEB128::ReadInt16(m_stream);
	item->hitPoints = LEB128::ReadInt16(m_stream);
}

bool SaveGame::readVariable()
{
	LuaVariable variable;
	
	variable.IsGlobal = LEB128::ReadByte(m_stream) == 1;
	variable.Type = LEB128::ReadInt32(m_stream);

	char* variableName;
	m_stream->ReadString(&variableName);
	variable.Name = string(variableName);
	free(variableName);

	if (variable.Type == LUA_VARIABLE_TYPE_BOOL)
		m_stream->ReadBool(&variable.BoolValue);
	else if (variable.Type == LUA_VARIABLE_TYPE_INT)
		m_stream->ReadInt32(&variable.IntValue);
	else if (variable.Type == LUA_VARIABLE_TYPE_FLOAT)
		m_stream->ReadFloat(&variable.FloatValue);
	else if (variable.Type == LUA_VARIABLE_TYPE_STRING)
	{
		char* variableValue;
		m_stream->ReadString(&variableValue);
		variable.StringValue = string(variableValue);
		free(variableValue);
	}
}

bool SaveGame::readSavegameChunks(ChunkId* chunkId, __int32 maxSize)
{
	if (chunkId->EqualsTo(m_chunkHeader))
		return readHeader();
	else if (chunkId->EqualsTo(m_chunkGameStatus))
		return readGameStatus();
	else if (chunkId->EqualsTo(m_chunkLara))
		return readLara();
	else if (chunkId->EqualsTo(m_chunkItem))
		return readItem();
	else if (chunkId->EqualsTo(m_chunkLuaVariable))
		return readVariable();

	return false;
}

bool SaveGame::Load(char* fileName)
{
	m_luaVariables.clear();

	m_stream = new FileStream(fileName);
	m_reader = new ChunkReader(0x4D355254, m_stream);
	m_reader->ReadChunks(&readSavegameChunks);
	m_stream->Close();
	delete m_writer;
	delete m_stream;

	g_GameScript->SetVariables(&m_luaVariables);

	return true;
}