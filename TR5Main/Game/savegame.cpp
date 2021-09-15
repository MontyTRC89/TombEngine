#include "framework.h"
#include "savegame.h"
#include "Lara.h"
#include "items.h"
#include "box.h"
#include "pickup.h"
#include "lot.h"
#include "switch.h"
#include "spotcam.h"
#include "traps.h"
#include "Sound\sound.h"
#include "level.h"
#include "setup.h"
#include "camera.h"
#include "flipeffect.h"
#include "quad.h"
#include "tr5_rats_emitter.h"
#include "tr5_bats_emitter.h"
#include "tr5_spider_emitter.h"
#include "generic_switch.h"
#include "fullblock_switch.h"
#include "creature_info.h"
#include "quad_info.h"

using namespace TEN::Entities::Switches;

using std::string;
using std::vector;
FileStream* SaveGame::m_stream;
ChunkReader* SaveGame::m_reader;
ChunkWriter* SaveGame::m_writer;
vector<LuaVariable> SaveGame::m_luaVariables;
int SaveGame::LastSaveGame;

std::unique_ptr<ChunkId> SaveGame::m_chunkGameStatus;
std::unique_ptr<ChunkId> SaveGame::m_chunkItems;
std::unique_ptr<ChunkId> SaveGame::m_chunkItem;
std::unique_ptr<ChunkId> SaveGame::m_chunkLara;
std::unique_ptr<ChunkId> SaveGame::m_chunkLuaVariable;
std::unique_ptr<ChunkId> SaveGame::m_chunkStaticFlags;
std::unique_ptr<ChunkId> SaveGame::m_chunkVehicle;
std::unique_ptr<ChunkId> SaveGame::m_chunkSequenceSwitch;
std::unique_ptr<ChunkId> SaveGame::m_chunkFlybyFlags;
std::unique_ptr<ChunkId> SaveGame::m_chunkCdFlags;
std::unique_ptr<ChunkId> SaveGame::m_chunkCamera;
std::unique_ptr<ChunkId> SaveGame::m_chunkFlipStats;
std::unique_ptr<ChunkId> SaveGame::m_chunkFlipMap;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemDummy;
std::unique_ptr<ChunkId> SaveGame::m_chunkStatistics;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemAnims;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemMeshes;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemFlags;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemHitPoints;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemPosition;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemIntelligentData;
std::unique_ptr<ChunkId> SaveGame::m_chunkSpecialItemBurningTorch;
std::unique_ptr<ChunkId> SaveGame::m_chunkSpecialItemChaff;
std::unique_ptr<ChunkId> SaveGame::m_chunkSpecialItemTorpedo;
std::unique_ptr<ChunkId> SaveGame::m_chunkSpecialItemCrossbowBolt;
std::unique_ptr<ChunkId> SaveGame::m_chunkSpecialItemFlare;
std::unique_ptr<ChunkId> SaveGame::m_chunkItemQuadInfo;
std::unique_ptr<ChunkId> SaveGame::m_chunkBats;
std::unique_ptr<ChunkId> SaveGame::m_chunkRats;
std::unique_ptr<ChunkId> SaveGame::m_chunkSpiders;
std::unique_ptr<ChunkId> SaveGame::m_chunkLaraExtraInfo;
std::unique_ptr<ChunkId> SaveGame::m_chunkWeaponInfo;
std::unique_ptr<ChunkId> SaveGame::m_chunkPuzzle;
std::unique_ptr<ChunkId> SaveGame::m_chunkKey;
std::unique_ptr<ChunkId> SaveGame::m_chunkPickup;
std::unique_ptr<ChunkId> SaveGame::m_chunkExamine;
std::unique_ptr<ChunkId> SaveGame::m_chunkPuzzleCombo;
std::unique_ptr<ChunkId> SaveGame::m_chunkKeyCombo;
std::unique_ptr<ChunkId> SaveGame::m_chunkPickupCombo;
std::unique_ptr<ChunkId> SaveGame::m_chunkExamineCombo;
std::unique_ptr<ChunkId> SaveGame::m_chunkWeaponItem;

SAVEGAME_INFO Savegame;
//extern vector<AudioTrack> g_AudioTracks;

void SaveGame::saveItems()
{
	// Save level items
	for (int i = 0; i < g_Level.NumItems; i++)
		m_writer->WriteChunkWithChildren(m_chunkItem.get(), &saveItem, i, 0);

	// Save items created at runtime (flares, missiles...)
	for (int i = g_Level.NumItems; i < NUM_ITEMS; i++)
	{
		ITEM_INFO* item = &g_Level.Items[i];
		if (item->active)
		{
			// Some items are very special and are saved in specific functions, all the others use the general function
			if (item->objectNumber == ID_BURNING_TORCH_ITEM)
				m_writer->WriteChunk(m_chunkSpecialItemBurningTorch.get(), &saveBurningTorch, i, 1);
			else if (item->objectNumber == ID_CHAFF)
				m_writer->WriteChunk(m_chunkSpecialItemChaff.get(), &saveChaff, i, 1);
			else if (item->objectNumber == ID_TORPEDO)
				m_writer->WriteChunk(m_chunkSpecialItemTorpedo.get(), &saveTorpedo, i, 1);
			else if (item->objectNumber == ID_CROSSBOW_BOLT)
				m_writer->WriteChunk(m_chunkSpecialItemCrossbowBolt.get(), &saveCrossbowBolt, i, 1);
			else if (item->objectNumber == ID_FLARE_ITEM)
				m_writer->WriteChunk(m_chunkSpecialItemFlare.get(), &saveFlare, i, 1);
			else
				m_writer->WriteChunkWithChildren(m_chunkItem.get(), &saveItem, i, 1);
		}
	}

	// Save special items
	if (Objects[ID_BATS_EMITTER].loaded)
		for (int i = 0; i < NUM_BATS; i++)
			if (Bats[i].on)
				m_writer->WriteChunk(m_chunkBats.get(), &saveBats, i, 0);

	if (Objects[ID_RATS_EMITTER].loaded)
		for (int i = 0; i < NUM_RATS; i++)
			if (Rats[i].on)
				m_writer->WriteChunk(m_chunkRats.get(), &saveRats, i, 0);

	if (Objects[ID_SPIDERS_EMITTER].loaded)
		for (int i = 0; i < NUM_SPIDERS; i++)
			if (Spiders[i].on)
				m_writer->WriteChunk(m_chunkSpiders.get(), &saveSpiders, i, 0);
}

void SaveGame::saveItem(int itemNumber, int runtimeItem)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	LEB128::Write(m_stream, itemNumber);
	LEB128::Write(m_stream, runtimeItem);

	bool hasData = true;

	if (item->flags & IFLAG_KILLED)
	{
		LEB128::Write(m_stream, 0x2000);
		hasData = false;
	}
	else if (item->flags & (IFLAG_ACTIVATION_MASK | IFLAG_INVISIBLE | 0x20) 
		|| item->objectNumber == ID_LARA)
	{
		LEB128::Write(m_stream, 0x8000);
		hasData = true;
	}
	else
	{
		LEB128::Write(m_stream, 0x0000);
		hasData = false;
	}

	LEB128::Write(m_stream, item->objectNumber);

	if (hasData)
	{
		m_writer->WriteChunkInt(m_chunkItemDummy.get(), 1);

		if (obj->saveAnim)
			m_writer->WriteChunk(m_chunkItemAnims.get(), &saveItemAnims, itemNumber, 0);

		if (obj->savePosition)
			m_writer->WriteChunk(m_chunkItemPosition.get(), &saveItemPosition, itemNumber, 0);

		if (obj->saveHitpoints)
			m_writer->WriteChunk(m_chunkItemHitPoints.get(), &saveItemHitPoints, itemNumber, 0);

		if (obj->saveFlags)
			m_writer->WriteChunkWithChildren(m_chunkItemFlags.get(), &saveItemFlags, itemNumber, 0);

		if (obj->saveMesh)
			m_writer->WriteChunk(m_chunkItemMeshes.get(), &saveItemMesh, itemNumber, 0);

		if (obj->intelligent && item->data != NULL)
			m_writer->WriteChunk(m_chunkItemIntelligentData.get(), &saveItemIntelligentData, itemNumber, 0);
	}
	else
	{
		m_writer->WriteChunkInt(m_chunkItemDummy.get(), 1);
	}
}

void SaveGame::saveGameStatus(int arg1, int arg2)
{
	/*LEB128::Write(m_stream, FlipStatus);
#ifndef NEW_INV
	LEB128::Write(m_stream, LastInventoryItem);
#endif
	LEB128::Write(m_stream, FlipEffect);
	LEB128::Write(m_stream, FlipTimer);
	//LEB128::Write(m_stream, CurrentAtmosphere);
	LEB128::Write(m_stream, CurrentSequence);

	// Now the sub-chunks
	for (int i = 0; i < g_Level.Rooms.size(); i++)
		for (int j = 0; j < g_Level.Rooms[i].mesh.size(); j++)
			m_writer->WriteChunk(m_chunkStaticFlags.get(), &saveStaticFlag, i, j);

	for (int i = 0; i < 6; i++)
		m_writer->WriteChunk(m_chunkSequenceSwitch.get(), &saveSequenceSwitch, i, SequenceUsed[i]);

	for (int i = 0; i < NumberSpotcams; i++)
		m_writer->WriteChunk(m_chunkFlybyFlags.get(), &saveFlybyFlags, i, SpotCam[i].flags);

	for (int i = 0; i < g_AudioTracks.size(); i++)
		m_writer->WriteChunk(m_chunkCdFlags.get(), &saveCdFlags, i, g_AudioTracks[i].Mask);

	for (int i = 0; i < NumberCameras; i++)
		m_writer->WriteChunk(m_chunkCamera.get(), &saveCamera, i, FixedCameras[i].flags);

	for (int i = 0; i < 255; i++)
		m_writer->WriteChunk(m_chunkFlipStats.get(), &saveFlipStats, i, FlipStats[i]);

	for (int i = 0; i < 255; i++)
		m_writer->WriteChunk(m_chunkFlipMap.get(), &saveFlipMap, i, FlipMap[i]);*/
}

void SaveGame::saveLara(int arg1, int arg2)
{
	// LARA_INFO struct dumped to savegame
	LaraInfo lara;
	memcpy(&lara, &Lara, sizeof(Lara));

	//lara.leftArm.frameBase = (short*)((char *)lara.leftArm.frameBase - (ptrdiff_t)Objects[ID_LARA].frameBase);
	//lara.rightArm.frameBase = (short*)((char *)lara.rightArm.frameBase - (ptrdiff_t)Objects[ID_LARA].frameBase);
	//lara.generalPtr = (char *)lara.generalPtr - (ptrdiff_t)malloc_buffer;

	m_stream->Write(reinterpret_cast<char*>(&lara), sizeof(Lara));
	
	// Lara weapon data
	if (Lara.weaponItem != NO_ITEM)
		m_writer->WriteChunk(m_chunkWeaponItem.get(), &saveWeaponItem, Lara.weaponItem, 0);
	
	// Save Lara extra info
	m_writer->WriteChunk(m_chunkLaraExtraInfo.get(), &saveLaraExtraInfo, 0, 0);
	
	// Save carried weapons
	for (int i = 0; i < NUM_WEAPONS; i++)
	{
		m_writer->WriteChunk(m_chunkWeaponInfo.get(), &saveWeaponInfo, i, 0);
	}

	// Save carried puzzles, keys, pickups and examines
	for (int i = 0; i < NUM_PUZZLES; i++)
	{
		if (Lara.Puzzles[i] > 0)
			m_writer->WriteChunk(m_chunkPuzzle.get(), &savePuzzle, i, Lara.Puzzles[i]);
	}

	for (int i = 0; i < NUM_PUZZLES * 2; i++)
	{
		if (Lara.PuzzlesCombo[i] > 0)
			m_writer->WriteChunk(m_chunkPuzzleCombo.get(), &savePuzzle, i, Lara.PuzzlesCombo[i]);
	}

	for (int i = 0; i < NUM_KEYS; i++)
	{
		if (Lara.Keys[i] > 0)
			m_writer->WriteChunk(m_chunkKey.get(), &savePuzzle, i, Lara.Keys[i]);
	}

	for (int i = 0; i < NUM_KEYS * 2; i++)
	{
		if (Lara.KeysCombo[i] > 0)
			m_writer->WriteChunk(m_chunkKeyCombo.get(), &savePuzzle, i, Lara.KeysCombo[i]);
	}

	for (int i = 0; i < NUM_PICKUPS; i++)
	{
		if (Lara.Pickups[i] > 0)
			m_writer->WriteChunk(m_chunkPickup.get(), &savePuzzle, i, Lara.Pickups[i]);
	}

	for (int i = 0; i < NUM_PICKUPS * 2; i++)
	{
		if (Lara.PickupsCombo[i] > 0)
			m_writer->WriteChunk(m_chunkPickupCombo.get(), &savePuzzle, i, Lara.PickupsCombo[i]);
	}

	for (int i = 0; i < NUM_EXAMINES; i++)
	{
		if (Lara.Examines[i] > 0)
			m_writer->WriteChunk(m_chunkExamine.get(), &savePuzzle, i, Lara.Examines[i]);
	}

	for (int i = 0; i < NUM_EXAMINES * 2; i++)
	{
		if (Lara.ExaminesCombo[i] > 0)
			m_writer->WriteChunk(m_chunkExamineCombo.get(), &savePuzzle, i, Lara.ExaminesCombo[i]);
	}
}

void SaveGame::saveWeaponItem(int arg1, int arg2)
{
	ITEM_INFO* weaponItem = &g_Level.Items[arg1];
	
	LEB128::Write(m_stream, weaponItem->objectNumber);
	LEB128::Write(m_stream, weaponItem->animNumber);
	LEB128::Write(m_stream, weaponItem->frameNumber);
	LEB128::Write(m_stream, weaponItem->currentAnimState);
	LEB128::Write(m_stream, weaponItem->goalAnimState);
	LEB128::Write(m_stream, weaponItem->requiredAnimState);
}

void SaveGame::saveLaraExtraInfo(int arg1, int arg2)
{
	LEB128::Write(m_stream, (Lara.Binoculars ? 1 : 0));
	LEB128::Write(m_stream, (Lara.Lasersight ? 1 : 0));
	LEB128::Write(m_stream, (Lara.Crowbar ? 1 : 0));
	LEB128::Write(m_stream, (Lara.Silencer ? 1 : 0));
	LEB128::Write(m_stream, (Lara.Torch ? 1 : 0));
	LEB128::Write(m_stream, Lara.Secrets);
	LEB128::Write(m_stream, Lara.ExtraAnim);
	LEB128::Write(m_stream, Lara.Vehicle);
	LEB128::Write(m_stream, Lara.mineL);
	LEB128::Write(m_stream, Lara.mineR);
	LEB128::Write(m_stream, Lara.NumFlares);
	LEB128::Write(m_stream, Lara.NumLargeMedipacks);
	LEB128::Write(m_stream, Lara.NumSmallMedipacks);
}

void SaveGame::savePuzzle(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1); // ID
	LEB128::Write(m_stream, arg2); // Quantity
}

void SaveGame::saveWeaponInfo(int arg1, int arg2)
{
	CarriedWeaponInfo* weapon = &Lara.Weapons[arg1];

	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, weapon->Present);
	LEB128::Write(m_stream, weapon->SelectedAmmo);
	LEB128::Write(m_stream, weapon->Ammo[WEAPON_AMMO1]);
	LEB128::Write(m_stream, weapon->Ammo[WEAPON_AMMO2]);
	LEB128::Write(m_stream, weapon->Ammo[WEAPON_AMMO3]);
	LEB128::Write(m_stream, weapon->HasSilencer);
	LEB128::Write(m_stream, weapon->HasLasersight);
}

void SaveGame::saveVariables()
{
	m_luaVariables.clear();
	//g_GameScript->GetVariables(&m_luaVariables);
	for (int i = 0; i < m_luaVariables.size(); i++)
		m_writer->WriteChunk(m_chunkLuaVariable.get(), &saveVariable, i, 0);
}

void SaveGame::saveVariable(int arg1, int arg2)
{
	LuaVariable variable = m_luaVariables[arg1];
	
	LEB128::Write(m_stream, variable.IsGlobal == 1);
	LEB128::Write(m_stream, variable.Type);
	m_stream->WriteString((char*)variable.Name.c_str());

	/*if (variable.Type == LUA_VARIABLE_TYPE_BOOL)
		m_stream->WriteBool(variable.BoolValue);
	else if (variable.Type == LUA_VARIABLE_TYPE_INT)
		m_stream->WriteInt32(variable.IntValue);
	else if (variable.Type == LUA_VARIABLE_TYPE_STRING)
		m_stream->WriteString((char*)variable.StringValue.c_str());
	else if (variable.Type == LUA_VARIABLE_TYPE_FLOAT)
		m_stream->WriteFloat(variable.FloatValue);*/
}

void SaveGame::Start()
{
	m_chunkGameStatus = ChunkId::FromString("TR5MSgGameStatus");
	m_chunkItems = ChunkId::FromString("TR5MSgItems");
	m_chunkItem = ChunkId::FromString("TR5MSgItem");
	m_chunkLara = ChunkId::FromString("TR5MSgLara");
	m_chunkLuaVariable = ChunkId::FromString("TR5MSgLuaVar");
	m_chunkStaticFlags = ChunkId::FromString("TR5MSgStFl");
	m_chunkVehicle = ChunkId::FromString("TR5MSgLaraVeh");
	m_chunkSequenceSwitch = ChunkId::FromString("TR5MSgSeqSw");
	m_chunkFlybyFlags = ChunkId::FromString("TR5MSgFlybyF");
	m_chunkCdFlags = ChunkId::FromString("TR5MSgCdF");
	m_chunkCamera = ChunkId::FromString("TR5MSgCam");
	m_chunkFlipStats = ChunkId::FromString("TR5MSgFlS");
	m_chunkFlipMap = ChunkId::FromString("TR5MSgFlM");
	m_chunkItemDummy = ChunkId::FromString("TR5MSgItemDummy");
	m_chunkStatistics = ChunkId::FromString("TR5MSgStats");
	m_chunkItemAnims = ChunkId::FromString("TR5MSgItAnms");
	m_chunkItemMeshes = ChunkId::FromString("TR5MSgItMsh");
	m_chunkItemFlags = ChunkId::FromString("TR5MSgItFl");
	m_chunkItemHitPoints = ChunkId::FromString("TR5MSgItHP");
	m_chunkItemPosition = ChunkId::FromString("TR5MSgItPos");
	m_chunkItemIntelligentData = ChunkId::FromString("TR5MSgItIntell");
	m_chunkSpecialItemBurningTorch = ChunkId::FromString("TR5MSgItTorch");
	m_chunkSpecialItemChaff = ChunkId::FromString("TR5MSgItChaff");
	m_chunkSpecialItemTorpedo = ChunkId::FromString("TR5MSgItTorpedo");
	m_chunkSpecialItemCrossbowBolt = ChunkId::FromString("TR5MSgItBolt");
	m_chunkSpecialItemFlare = ChunkId::FromString("TR5MSgItFlare");
	m_chunkItemQuadInfo = ChunkId::FromString("TR5MSgQuadInfo");
	m_chunkBats = ChunkId::FromString("TR5MSgBats");
	m_chunkRats = ChunkId::FromString("TR5MSgRats");
	m_chunkSpiders = ChunkId::FromString("TR5MSgSpiders");
	m_chunkLaraExtraInfo = ChunkId::FromString("TR5MSgLaraExtraInfo");
	m_chunkWeaponInfo = ChunkId::FromString("TR5MSgWeapon");
	m_chunkPuzzle = ChunkId::FromString("TR5MSgPuzzle");
	m_chunkPuzzleCombo = ChunkId::FromString("TR5MSgPuzzleC");
	m_chunkKey = ChunkId::FromString("TR5MSgKey");
	m_chunkKeyCombo = ChunkId::FromString("TR5MSgKeyC");
	m_chunkPickup = ChunkId::FromString("TR5MSgPickup");
	m_chunkPickupCombo = ChunkId::FromString("TR5MSgPickupC");
	m_chunkExamine = ChunkId::FromString("TR5MSgExamine");
	m_chunkExamineCombo = ChunkId::FromString("TR5MSgExamineC");
	m_chunkWeaponItem = ChunkId::FromString("TR5MSgWeaponItem");

	LastSaveGame = 0;
}

void SaveGame::End()
{
}

bool SaveGame::Save(char* fileName)
{
	m_stream = new FileStream(fileName, true, true);
	m_writer = new ChunkWriter(0x4D355254, m_stream);

	printf("Timer: %d\n", Savegame.Game.Timer);

	// The header must be here, so no chunks
	m_stream->WriteString(g_GameFlow->GetString(g_GameFlow->GetLevel(CurrentLevel)->NameStringKey.c_str()));
	LEB128::Write(m_stream, (GameTimer / 30) / 86400);
	LEB128::Write(m_stream, ((GameTimer / 30) % 86400) / 3600);
	LEB128::Write(m_stream, ((GameTimer / 30) / 60) % 60);
	LEB128::Write(m_stream, (GameTimer / 30) % 60);
	LEB128::Write(m_stream, CurrentLevel);
	LEB128::Write(m_stream, GameTimer);
	LEB128::Write(m_stream, ++LastSaveGame);

	// Now we write chunks
	m_writer->WriteChunk(m_chunkStatistics.get(), &saveStatistics, 0, 0);
	m_writer->WriteChunkWithChildren(m_chunkGameStatus.get(), &saveGameStatus, 0, 0);
	m_writer->WriteChunkWithChildren(m_chunkLara.get(), &saveLara, 0, 0);
	saveItems();
	saveVariables();
	
	// EOF
	m_stream->WriteInt32(0);

	m_stream->Close();

	delete m_writer;
	delete m_stream;

	return true;
}

bool SaveGame::readGameStatus()
{
	FlipStatus = LEB128::ReadInt32(m_stream);
#ifndef NEW_INV
	LastInventoryItem = LEB128::ReadInt32(m_stream);
#endif
	FlipEffect = LEB128::ReadInt32(m_stream);
	CurrentAtmosphere = LEB128::ReadByte(m_stream);
	CurrentSequence = LEB128::ReadByte(m_stream);

	m_reader->ReadChunks(&readGameStatusChunks, 0);

	return true;
}

bool SaveGame::readLara()
{
	// Read dumped LARA_INFO struct
	char* buffer = (char*)malloc(sizeof(LaraInfo));
	m_stream->Read(buffer, sizeof(LaraInfo));
	LaraInfo* lara = reinterpret_cast<LaraInfo*>(buffer);
	memcpy(&Lara, lara, sizeof(LaraInfo));
	free(buffer);

	//Lara.leftArm.frameBase = AddPtr(Lara.leftArm.frameBase, short, Objects[ID_LARA].frameBase);
	//Lara.rightArm.frameBase = AddPtr(Lara.rightArm.frameBase, short, Objects[ID_LARA].frameBase);
	
	Lara.target = NULL;
	Lara.spazEffect = NULL;
	//Lara.generalPtr = AddPtr(Lara.generalPtr, char, malloc_buffer);
	Lara.weaponItem = NO_ITEM;

	// Is Lara burning?
	if (Lara.burn)
	{
		Lara.burn = false;
		bool smokeFlag = false;
		if (Lara.burnSmoke)
		{
			Lara.burnSmoke = false;
			smokeFlag = true;
		}
		
		LaraBurn();
		if (smokeFlag)
			Lara.burnSmoke = true;
	}
	
	m_reader->ReadChunks(&readLaraChunks, 0);
	return true;
}

bool SaveGame::readItem()
{
	short itemNumber = LEB128::ReadInt16(m_stream);
	short runtimeItem = LEB128::ReadByte(m_stream);
	short itemKind = LEB128::ReadInt16(m_stream);

	// Runtime items must be allocated dynamically
	if (runtimeItem)
		itemNumber = CreateItem();
	
	ITEM_INFO* item = &g_Level.Items[itemNumber];
	item->objectNumber = from_underlying(LEB128::ReadInt16(m_stream));

	OBJECT_INFO* obj = &Objects[item->objectNumber];

	// Runtime items must be initialised
	// TODO: test test test!!!
	if (runtimeItem)
	{
		InitialiseItem(itemNumber);
		AddActiveItem(itemNumber);
	}

	if (itemKind == 0x2000)
	{
		m_reader->ReadChunks(&readItemChunks, itemNumber);
		DisableBaddieAI(itemNumber);
		KillItem(itemNumber);
		item->status = ITEM_DEACTIVATED;
		item->flags |= ONESHOT;
		item->afterDeath = 128;
	}
	else if (itemKind == 0x8000)
	{
		m_reader->ReadChunks(&readItemChunks, itemNumber);
	}
	else
	{
		m_reader->ReadChunks(&readItemChunks, itemNumber);
	}

	// Some post-processing things
	if (obj->isPuzzleHole && (item->status == ITEM_DEACTIVATED || item->status == ITEM_ACTIVE))
		item->objectNumber += GAME_OBJECT_ID{ NUM_PUZZLES };

	if (item->objectNumber >= ID_SMASH_OBJECT1 && item->objectNumber <= ID_SMASH_OBJECT8 && (item->flags & ONESHOT))
		item->meshBits = 0x100;

	if (item->objectNumber == ID_RAISING_BLOCK1 && item->itemFlags[1])
	{
		//if (item->triggerFlags == -1)
		//	AlterFloorHeight(item, -255);
		//else if (item->triggerFlags == -3)
		//	AlterFloorHeight(item, -1023);
		//else
		//	AlterFloorHeight(item, -1024);
	}

	//if (item->objectNumber == ID_RAISING_BLOCK2 && item->itemFlags[1])
	//	AlterFloorHeight(item, -2048);
	   
	return true;
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

	/*if (variable.Type == LUA_VARIABLE_TYPE_BOOL)
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
	}*/

	return true;
}

bool SaveGame::readSavegameChunks(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(m_chunkGameStatus.get()))
		return readGameStatus();
	else if (chunkId->EqualsTo(m_chunkLara.get()))
		return readLara();
	else if (chunkId->EqualsTo(m_chunkItem.get()))
		return readItem();
	else if (chunkId->EqualsTo(m_chunkLuaVariable.get()))
		return readVariable();
	else if (chunkId->EqualsTo(m_chunkStatistics.get()))
		return readStatistics();
	else if (chunkId->EqualsTo(m_chunkSpecialItemBurningTorch.get()))
		return readBurningTorch();
	else if (chunkId->EqualsTo(m_chunkSpecialItemChaff.get()))
		return readChaff();
	else if (chunkId->EqualsTo(m_chunkSpecialItemTorpedo.get()))
		return readTorpedo();
	else if (chunkId->EqualsTo(m_chunkSpecialItemCrossbowBolt.get()))
		return readCrossbowBolt();
	else if (chunkId->EqualsTo(m_chunkSpecialItemFlare.get()))
		return readFlare();
	else if (chunkId->EqualsTo(m_chunkBats.get()))
		return readBats();
	else if (chunkId->EqualsTo(m_chunkRats.get()))
		return readRats();
	else if (chunkId->EqualsTo(m_chunkSpiders.get()))
		return readSpiders();

	return false;
}

bool SaveGame::Load(char* fileName)
{
	m_luaVariables.clear();

	m_stream = new FileStream(fileName, true, false);
	m_reader = new ChunkReader(0x4D355254, m_stream);

	// Header must be here
	char* levelName;
	m_stream->ReadString(&levelName);

	// Skip timer
	LEB128::ReadInt32(m_stream);
	LEB128::ReadInt32(m_stream);
	LEB128::ReadInt32(m_stream);
	LEB128::ReadInt32(m_stream);

	CurrentLevel = LEB128::ReadByte(m_stream);
	GameTimer = LEB128::ReadInt32(m_stream);
	LastSaveGame = LEB128::ReadInt32(m_stream);

	// Read chunks
	m_reader->ReadChunks(&readSavegameChunks, 0);

	// Close the stream
	m_stream->Close();
	//delete m_writer;
	//delete m_stream;

	//g_GameScript->SetVariables(&m_luaVariables);

	JustLoaded = true;

	return true;
}

bool SaveGame::LoadHeader(char* fileName, SaveGameHeader* header)
{
	m_stream = new FileStream(fileName, true, false);
	m_reader = new ChunkReader(0x4D355254, m_stream);

	// Header must be here
	char* levelName;
	m_stream->ReadString(&levelName);
	header->LevelName = string(levelName);
	free(levelName);

	// Skip timer
	header->Days = LEB128::ReadInt32(m_stream);
	header->Hours = LEB128::ReadInt32(m_stream);
	header->Minutes = LEB128::ReadInt32(m_stream);
	header->Seconds = LEB128::ReadInt32(m_stream);
	
	header->Level = LEB128::ReadByte(m_stream);
	header->Timer = LEB128::ReadInt32(m_stream);
	header->Count = LEB128::ReadInt32(m_stream);

	// Close the stream
	m_stream->Close();
	//delete m_writer;
	//delete m_stream;

	return true;
}

void SaveGame::saveStaticFlag(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, arg2);
	LEB128::Write(m_stream, g_Level.Rooms[arg1].mesh[arg2].flags);
}

bool SaveGame::readLaraChunks(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(m_chunkLaraExtraInfo.get()))
	{
		Lara.Binoculars = LEB128::ReadByte(m_stream);
		Lara.Lasersight = LEB128::ReadByte(m_stream);
		Lara.Crowbar = LEB128::ReadByte(m_stream);
		Lara.Silencer = LEB128::ReadByte(m_stream);
		Lara.Torch = LEB128::ReadByte(m_stream);
		Lara.Secrets = LEB128::ReadInt32(m_stream);
		Lara.ExtraAnim = LEB128::ReadInt16(m_stream);
		Lara.Vehicle = LEB128::ReadInt16(m_stream);
		Lara.mineL = LEB128::ReadByte(m_stream);
		Lara.mineR = LEB128::ReadByte(m_stream);
		Lara.NumFlares = LEB128::ReadInt32(m_stream);
		Lara.NumLargeMedipacks = LEB128::ReadInt32(m_stream);
		Lara.NumSmallMedipacks = LEB128::ReadInt32(m_stream);

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkWeaponInfo.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		CarriedWeaponInfo* weapon = &Lara.Weapons[id];

		weapon->Present = LEB128::ReadByte(m_stream);
		weapon->SelectedAmmo = LEB128::ReadByte(m_stream);
		weapon->Ammo[WEAPON_AMMO1] = LEB128::ReadInt16(m_stream);
		weapon->Ammo[WEAPON_AMMO2] = LEB128::ReadInt16(m_stream);
		weapon->Ammo[WEAPON_AMMO3] = LEB128::ReadInt16(m_stream);
		weapon->HasSilencer = LEB128::ReadByte(m_stream);
		weapon->HasLasersight = LEB128::ReadByte(m_stream);
	}
	else if (chunkId->EqualsTo(m_chunkPuzzle.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.Puzzles[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkPuzzleCombo.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.PuzzlesCombo[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkKey.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.Keys[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkKeyCombo.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.KeysCombo[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkPickup.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.Pickups[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkPickupCombo.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.PickupsCombo[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkExamine.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.Examines[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkExamineCombo.get()))
	{
		int id = LEB128::ReadInt32(m_stream);
		int quantity = LEB128::ReadInt32(m_stream);
		Lara.ExaminesCombo[id] = quantity;
	}
	else if (chunkId->EqualsTo(m_chunkWeaponItem.get()))
	{
		short weaponItemNum = CreateItem();
		Lara.weaponItem = weaponItemNum;

		ITEM_INFO* weaponItem = &g_Level.Items[Lara.weaponItem];

		weaponItem->objectNumber = from_underlying(LEB128::ReadInt16(m_stream));
		weaponItem->animNumber = LEB128::ReadInt16(m_stream);
		weaponItem->frameNumber = LEB128::ReadInt16(m_stream);
		weaponItem->currentAnimState = LEB128::ReadInt16(m_stream);
		weaponItem->goalAnimState = LEB128::ReadInt16(m_stream);
		weaponItem->requiredAnimState = LEB128::ReadInt16(m_stream);
		weaponItem->roomNumber = 255;
	}

	return false;
}

bool SaveGame::readGameStatusChunks(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(m_chunkStaticFlags.get()))
	{
		short roomIndex = LEB128::ReadInt16(m_stream);
		short staticIndex = LEB128::ReadInt16(m_stream);
		short flags = LEB128::ReadInt16(m_stream);
		g_Level.Rooms[roomIndex].mesh[staticIndex].flags = flags;

		if (!flags)
		{
			FLOOR_INFO* floor = GetFloor(g_Level.Rooms[roomIndex].mesh[staticIndex].pos.xPos,
										 g_Level.Rooms[roomIndex].mesh[staticIndex].pos.yPos,
										 g_Level.Rooms[roomIndex].mesh[staticIndex].pos.zPos,
										 &roomIndex);

			TestTriggers(g_Level.Rooms[roomIndex].mesh[staticIndex].pos.xPos,
						 g_Level.Rooms[roomIndex].mesh[staticIndex].pos.yPos,
						 g_Level.Rooms[roomIndex].mesh[staticIndex].pos.zPos, roomIndex, true);

			floor->Stopper = false;
		}

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkFlipStats.get()))
	{
		short index = LEB128::ReadInt16(m_stream);
		short value = LEB128::ReadInt16(m_stream);
		//FlipStats[index] = value;
		return true;
	}
	else if (chunkId->EqualsTo(m_chunkFlipMap.get()))
	{
		short index = LEB128::ReadInt16(m_stream);
		short value = LEB128::ReadInt16(m_stream);
		FlipMap[index] = value;
		if ((value & 0x3E00) == 0x3E00)
			DoFlipMap(index);
		return true;
	}
	/*else if (chunkId->EqualsTo(m_chunkCdFlags.get()))
	{
		short index = LEB128::ReadInt16(m_stream);
		printf("Index: %d\n", index);
		short value = LEB128::ReadInt16(m_stream);
		if (index < g_AudioTracks.size())
			g_AudioTracks[index].Mask = value;
		return true;
	}*/
	else if (chunkId->EqualsTo(m_chunkCamera.get()))
	{
		short index = LEB128::ReadInt16(m_stream);
		short value = LEB128::ReadInt16(m_stream);
		g_Level.Cameras[index].flags = value;
		return true;
	}
	else if (chunkId->EqualsTo(m_chunkSequenceSwitch.get()))
	{
		short index = LEB128::ReadInt16(m_stream);
		short value = LEB128::ReadInt16(m_stream);
		SequenceUsed[index] = value;
		return true;
	}
	else if (chunkId->EqualsTo(m_chunkFlybyFlags.get()))
	{
		int index = LEB128::ReadInt16(m_stream);
		int value = LEB128::ReadInt16(m_stream);
		SpotCam[index].flags = value;
		return true;
	}
	return false;
}

void SaveGame::saveCamera(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, arg2);
}

void SaveGame::saveSequenceSwitch(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, arg2);
}

void SaveGame::saveFlybyFlags(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, arg2);
}

void SaveGame::saveFlipMap(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, arg2);
}

void SaveGame::saveFlipStats(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, arg2);
}

void SaveGame::saveCdFlags(int arg1, int arg2)
{
	LEB128::Write(m_stream, arg1);
	LEB128::Write(m_stream, arg2);
}

bool SaveGame::readItemChunks(ChunkId* chunkId, int maxSize, int itemNumber)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	if (chunkId->EqualsTo(m_chunkItemDummy.get()))
		return m_reader->ReadChunkInt32(maxSize);
	else if (chunkId->EqualsTo(m_chunkItemAnims.get()))
	{
		item->currentAnimState = LEB128::ReadInt16(m_stream);
		item->goalAnimState = LEB128::ReadInt16(m_stream);
		item->requiredAnimState = LEB128::ReadInt16(m_stream);
		item->animNumber = LEB128::ReadInt16(m_stream);
		item->frameNumber = LEB128::ReadInt16(m_stream);

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkItemPosition.get()))
	{
		item->pos.xPos = LEB128::ReadInt32(m_stream);
		item->pos.yPos = LEB128::ReadInt32(m_stream);
		item->pos.zPos = LEB128::ReadInt32(m_stream);
		item->pos.xRot = LEB128::ReadInt16(m_stream);
		item->pos.yRot = LEB128::ReadInt16(m_stream);
		item->pos.zRot = LEB128::ReadInt16(m_stream);

		short roomNumber = LEB128::ReadInt16(m_stream);
		if (item->roomNumber != roomNumber)
			ItemNewRoom(itemNumber, roomNumber);

		item->speed = LEB128::ReadInt16(m_stream);
		item->fallspeed = LEB128::ReadInt16(m_stream);

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkItemHitPoints.get()))
	{
		item->hitPoints = LEB128::ReadInt16(m_stream);

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkItemFlags.get()))
	{
		item->flags = LEB128::ReadInt16(m_stream);
		byte active = LEB128::ReadByte(m_stream);
		item->status = LEB128::ReadByte(m_stream);
		item->gravityStatus = LEB128::ReadByte(m_stream);
		item->hitStatus = LEB128::ReadByte(m_stream);
		item->collidable = LEB128::ReadByte(m_stream);
		item->lookedAt = LEB128::ReadByte(m_stream);
		item->dynamicLight = LEB128::ReadByte(m_stream);
		item->poisoned = LEB128::ReadByte(m_stream);
		item->aiBits = LEB128::ReadByte(m_stream);
		item->reallyActive = LEB128::ReadByte(m_stream);
		item->triggerFlags = LEB128::ReadInt16(m_stream);
		item->timer = LEB128::ReadByte(m_stream);
		item->itemFlags[0] = LEB128::ReadInt16(m_stream);
		item->itemFlags[1] = LEB128::ReadInt16(m_stream);
		item->itemFlags[2] = LEB128::ReadInt16(m_stream);
		item->itemFlags[3] = LEB128::ReadInt16(m_stream);

		if (active && !item->active)
			AddActiveItem(itemNumber);

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkItemIntelligentData.get()))
	{
		EnableBaddieAI(itemNumber, 1);

		OBJECT_INFO* obj = &Objects[item->objectNumber];
		CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

		creature->jointRotation[0] = LEB128::ReadInt16(m_stream);
		creature->jointRotation[1] = LEB128::ReadInt16(m_stream);
		creature->jointRotation[2] = LEB128::ReadInt16(m_stream);
		creature->jointRotation[3] = LEB128::ReadInt16(m_stream);
		creature->maximumTurn = LEB128::ReadInt16(m_stream);
		creature->flags = LEB128::ReadInt16(m_stream);
		creature->alerted = LEB128::ReadByte(m_stream);
		creature->headLeft = LEB128::ReadByte(m_stream);
		creature->headRight = LEB128::ReadByte(m_stream);
		creature->reachedGoal = LEB128::ReadByte(m_stream);
		creature->hurtByLara = LEB128::ReadByte(m_stream);
		creature->patrol2 = LEB128::ReadByte(m_stream);
		creature->jumpAhead = LEB128::ReadByte(m_stream);
		creature->monkeyAhead = LEB128::ReadByte(m_stream);
		creature->mood = (MOOD_TYPE)LEB128::ReadInt32(m_stream);

		ITEM_INFO* enemy = (ITEM_INFO*)LEB128::ReadLong(m_stream);
		//creature->enemy = AddPtr(enemy, ITEM_INFO, malloc_buffer);

		creature->aiTarget.objectNumber = from_underlying(LEB128::ReadInt16(m_stream));
		creature->aiTarget.roomNumber = LEB128::ReadInt16(m_stream);
		creature->aiTarget.boxNumber = LEB128::ReadInt16(m_stream);
		creature->aiTarget.flags = LEB128::ReadInt16(m_stream);
		creature->aiTarget.pos.xPos = LEB128::ReadInt32(m_stream);
		creature->aiTarget.pos.yPos = LEB128::ReadInt32(m_stream);
		creature->aiTarget.pos.zPos = LEB128::ReadInt32(m_stream);
		creature->aiTarget.pos.xRot = LEB128::ReadInt16(m_stream);
		creature->aiTarget.pos.yRot = LEB128::ReadInt16(m_stream);
		creature->aiTarget.pos.zRot = LEB128::ReadInt16(m_stream);

		creature->LOT.canJump = LEB128::ReadByte(m_stream);
		creature->LOT.canMonkey = LEB128::ReadByte(m_stream);
		creature->LOT.isAmphibious = LEB128::ReadByte(m_stream);
		creature->LOT.isJumping = LEB128::ReadByte(m_stream);
		creature->LOT.isMonkeying = LEB128::ReadByte(m_stream);

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkItemQuadInfo.get()))
	{
		//QUAD_INFO* quadInfo = game_malloc<QUAD_INFO>();
		//m_stream->ReadBytes(reinterpret_cast<byte*>(quadInfo), sizeof(QUAD_INFO));
		//if (item->objectNumber == ID_QUAD)
		//	item->data = quadInfo;

		return true;
	}
	else if (chunkId->EqualsTo(m_chunkItemMeshes.get()))
	{
		item->meshBits = LEB128::ReadInt32(m_stream);
		item->swapMeshFlags = LEB128::ReadInt32(m_stream);

		return true;
	}

	return false;
}

void SaveGame::saveStatistics(int arg1, int arg2)
{
	LEB128::Write(m_stream, Savegame.Game.AmmoHits);
	LEB128::Write(m_stream, Savegame.Game.AmmoUsed);
	LEB128::Write(m_stream, Savegame.Game.Distance);
	LEB128::Write(m_stream, Savegame.Game.HealthUsed);
	LEB128::Write(m_stream, Savegame.Game.Kills);
	LEB128::Write(m_stream, Savegame.Game.Secrets);
	LEB128::Write(m_stream, Savegame.Game.Timer);

	LEB128::Write(m_stream, Savegame.Level.AmmoHits);
	LEB128::Write(m_stream, Savegame.Level.AmmoUsed);
	LEB128::Write(m_stream, Savegame.Level.Distance);
	LEB128::Write(m_stream, Savegame.Level.HealthUsed);
	LEB128::Write(m_stream, Savegame.Level.Kills);
	LEB128::Write(m_stream, Savegame.Level.Secrets);
	LEB128::Write(m_stream, Savegame.Level.Timer);
}

bool SaveGame::readStatistics()
{
	Savegame.Game.AmmoHits = LEB128::ReadInt32(m_stream);
	Savegame.Game.AmmoUsed = LEB128::ReadInt32(m_stream);
	Savegame.Game.Distance = LEB128::ReadInt32(m_stream);
	Savegame.Game.HealthUsed = LEB128::ReadByte(m_stream);
	Savegame.Game.Kills = LEB128::ReadInt16(m_stream);
	Savegame.Game.Secrets = LEB128::ReadByte(m_stream);
	Savegame.Game.Timer = LEB128::ReadInt32(m_stream);

	Savegame.Level.AmmoHits = LEB128::ReadInt32(m_stream);
	Savegame.Level.AmmoUsed = LEB128::ReadInt32(m_stream);
	Savegame.Level.Distance = LEB128::ReadInt32(m_stream);
	Savegame.Level.HealthUsed = LEB128::ReadByte(m_stream);
	Savegame.Level.Kills = LEB128::ReadInt16(m_stream);
	Savegame.Level.Secrets = LEB128::ReadByte(m_stream);
	Savegame.Level.Timer = LEB128::ReadInt32(m_stream);

	return true;
}

void SaveGame::saveItemFlags(int arg1, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[arg1];
	OBJECT_INFO* obj = &Objects[item->objectNumber];

	LEB128::Write(m_stream, item->flags);
	LEB128::Write(m_stream, item->active);
	LEB128::Write(m_stream, item->status);
	LEB128::Write(m_stream, item->gravityStatus);
	LEB128::Write(m_stream, item->hitStatus);
	LEB128::Write(m_stream, item->collidable);
	LEB128::Write(m_stream, item->lookedAt);
	LEB128::Write(m_stream, item->dynamicLight);
	LEB128::Write(m_stream, item->poisoned);
	LEB128::Write(m_stream, item->aiBits);
	LEB128::Write(m_stream, item->reallyActive);
	LEB128::Write(m_stream, item->triggerFlags);
	LEB128::Write(m_stream, item->timer);
	LEB128::Write(m_stream, item->itemFlags[0]);
	LEB128::Write(m_stream, item->itemFlags[1]);
	LEB128::Write(m_stream, item->itemFlags[2]);
	LEB128::Write(m_stream, item->itemFlags[3]);
}

void SaveGame::saveItemIntelligentData(int arg1, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[arg1];
	OBJECT_INFO* obj = &Objects[item->objectNumber];
	CREATURE_INFO* creature = (CREATURE_INFO*)item->data;

	ITEM_INFO* enemy = (ITEM_INFO*)((char*)creature->enemy);

	LEB128::Write(m_stream, creature->jointRotation[0]);
	LEB128::Write(m_stream, creature->jointRotation[1]);
	LEB128::Write(m_stream, creature->jointRotation[2]);
	LEB128::Write(m_stream, creature->jointRotation[3]);
	LEB128::Write(m_stream, creature->maximumTurn);
	LEB128::Write(m_stream, creature->flags);
	LEB128::Write(m_stream, creature->alerted);
	LEB128::Write(m_stream, creature->headLeft);
	LEB128::Write(m_stream, creature->headRight);
	LEB128::Write(m_stream, creature->reachedGoal);
	LEB128::Write(m_stream, creature->hurtByLara);
	LEB128::Write(m_stream, creature->patrol2);
	LEB128::Write(m_stream, creature->jumpAhead);
	LEB128::Write(m_stream, creature->monkeyAhead);
	LEB128::Write(m_stream, creature->mood);
	LEB128::Write(m_stream, (int)enemy);

	LEB128::Write(m_stream, creature->aiTarget.objectNumber);
	LEB128::Write(m_stream, creature->aiTarget.roomNumber);
	LEB128::Write(m_stream, creature->aiTarget.boxNumber);
	LEB128::Write(m_stream, creature->aiTarget.flags);
	LEB128::Write(m_stream, creature->aiTarget.pos.xPos);
	LEB128::Write(m_stream, creature->aiTarget.pos.yPos);
	LEB128::Write(m_stream, creature->aiTarget.pos.zPos);
	LEB128::Write(m_stream, creature->aiTarget.pos.xRot);
	LEB128::Write(m_stream, creature->aiTarget.pos.yRot);
	LEB128::Write(m_stream, creature->aiTarget.pos.zRot);

	LEB128::Write(m_stream, creature->LOT.canJump);
	LEB128::Write(m_stream, creature->LOT.canMonkey);
	LEB128::Write(m_stream, creature->LOT.isAmphibious);
	LEB128::Write(m_stream, creature->LOT.isJumping);
	LEB128::Write(m_stream, creature->LOT.isMonkeying);
}

void SaveGame::saveItemHitPoints(int arg1, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[arg1];

	LEB128::Write(m_stream, item->hitPoints);
}

void SaveGame::saveItemPosition(int arg1, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[arg1];

	LEB128::Write(m_stream, item->pos.xPos);
	LEB128::Write(m_stream, item->pos.yPos);
	LEB128::Write(m_stream, item->pos.zPos);
	LEB128::Write(m_stream, item->pos.xRot);
	LEB128::Write(m_stream, item->pos.yRot);
	LEB128::Write(m_stream, item->pos.zRot);
	LEB128::Write(m_stream, item->roomNumber);
	LEB128::Write(m_stream, item->speed);
	LEB128::Write(m_stream, item->fallspeed);
}

void SaveGame::saveItemMesh(int arg1, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[arg1];

	LEB128::Write(m_stream, item->meshBits);
	LEB128::Write(m_stream, item->swapMeshFlags);
}

void SaveGame::saveItemAnims(int arg1, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[arg1];

	LEB128::Write(m_stream, item->currentAnimState);
	LEB128::Write(m_stream, item->goalAnimState);
	LEB128::Write(m_stream, item->requiredAnimState);
	LEB128::Write(m_stream, item->animNumber);
	LEB128::Write(m_stream, item->frameNumber);
}

void SaveGame::saveBurningTorch(int itemNumber, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	LEB128::Write(m_stream, itemNumber);
	LEB128::Write(m_stream, item->pos.xPos);
	LEB128::Write(m_stream, item->pos.yPos);
	LEB128::Write(m_stream, item->pos.zPos);
	LEB128::Write(m_stream, item->pos.xRot);
	LEB128::Write(m_stream, item->pos.yRot);
	LEB128::Write(m_stream, item->pos.zRot);
	LEB128::Write(m_stream, item->roomNumber);
	LEB128::Write(m_stream, item->speed);
	LEB128::Write(m_stream, item->fallspeed);

	LEB128::Write(m_stream, item->itemFlags[2]);
}

void SaveGame::saveChaff(int itemNumber, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	LEB128::Write(m_stream, itemNumber);
	LEB128::Write(m_stream, item->pos.xPos);
	LEB128::Write(m_stream, item->pos.yPos);
	LEB128::Write(m_stream, item->pos.zPos);
	LEB128::Write(m_stream, item->pos.xRot);
	LEB128::Write(m_stream, item->pos.yRot);
	LEB128::Write(m_stream, item->pos.zRot);
	LEB128::Write(m_stream, item->roomNumber);
	LEB128::Write(m_stream, item->speed);
	LEB128::Write(m_stream, item->fallspeed);

	LEB128::Write(m_stream, item->itemFlags[0]);
	LEB128::Write(m_stream, item->itemFlags[1]);
}

void SaveGame::saveTorpedo(int itemNumber, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	LEB128::Write(m_stream, itemNumber);
	LEB128::Write(m_stream, item->pos.xPos);
	LEB128::Write(m_stream, item->pos.yPos);
	LEB128::Write(m_stream, item->pos.zPos);
	LEB128::Write(m_stream, item->pos.xRot);
	LEB128::Write(m_stream, item->pos.yRot);
	LEB128::Write(m_stream, item->pos.zRot);
	LEB128::Write(m_stream, item->roomNumber);
	LEB128::Write(m_stream, item->speed);
	LEB128::Write(m_stream, item->fallspeed);

	LEB128::Write(m_stream, item->itemFlags[0]);
	LEB128::Write(m_stream, item->itemFlags[1]);
	LEB128::Write(m_stream, item->currentAnimState);
	LEB128::Write(m_stream, item->goalAnimState);
	LEB128::Write(m_stream, item->requiredAnimState);
}

void SaveGame::saveCrossbowBolt(int itemNumber, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	LEB128::Write(m_stream, itemNumber);
	LEB128::Write(m_stream, item->pos.xPos);
	LEB128::Write(m_stream, item->pos.yPos);
	LEB128::Write(m_stream, item->pos.zPos);
	LEB128::Write(m_stream, item->pos.xRot);
	LEB128::Write(m_stream, item->pos.yRot);
	LEB128::Write(m_stream, item->pos.zRot);
	LEB128::Write(m_stream, item->roomNumber);
	LEB128::Write(m_stream, item->speed);
	LEB128::Write(m_stream, item->fallspeed);
}

void SaveGame::saveFlare(int itemNumber, int arg2)
{
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	LEB128::Write(m_stream, itemNumber);
	LEB128::Write(m_stream, item->pos.xPos);
	LEB128::Write(m_stream, item->pos.yPos);
	LEB128::Write(m_stream, item->pos.zPos);
	LEB128::Write(m_stream, item->pos.xRot);
	LEB128::Write(m_stream, item->pos.yRot);
	LEB128::Write(m_stream, item->pos.zRot);
	LEB128::Write(m_stream, item->roomNumber);
	LEB128::Write(m_stream, item->speed);
	LEB128::Write(m_stream, item->fallspeed);

	// Flare age
	LEB128::Write(m_stream, (int)item->data);
}

bool SaveGame::readBurningTorch()
{
	LEB128::ReadInt16(m_stream);
	
	short itemNumber = CreateItem();
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->objectNumber = ID_BURNING_TORCH_ITEM;
	item->pos.xPos = LEB128::ReadInt32(m_stream);
	item->pos.yPos = LEB128::ReadInt32(m_stream);
	item->pos.zPos = LEB128::ReadInt32(m_stream);
	item->pos.xRot = LEB128::ReadInt16(m_stream);
	item->pos.yRot = LEB128::ReadInt16(m_stream);
	item->pos.zRot = LEB128::ReadInt16(m_stream);
	item->roomNumber = LEB128::ReadInt16(m_stream);

	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	InitialiseItem(itemNumber);

	item->pos.xRot = oldXrot;
	item->pos.yRot = oldYrot;
	item->pos.zRot = oldZrot;

	item->speed = LEB128::ReadInt16(m_stream);
	item->fallspeed = LEB128::ReadInt16(m_stream);

	AddActiveItem(itemNumber);

	item->itemFlags[2] = LEB128::ReadInt16(m_stream);

	return true;
}

bool SaveGame::readChaff()
{
	LEB128::ReadInt16(m_stream);

	short itemNumber = CreateItem();
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->objectNumber = ID_CHAFF;
	item->pos.xPos = LEB128::ReadInt32(m_stream);
	item->pos.yPos = LEB128::ReadInt32(m_stream);
	item->pos.zPos = LEB128::ReadInt32(m_stream);
	item->pos.xRot = LEB128::ReadInt16(m_stream);
	item->pos.yRot = LEB128::ReadInt16(m_stream);
	item->pos.zRot = LEB128::ReadInt16(m_stream);
	item->roomNumber = LEB128::ReadInt16(m_stream);

	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	InitialiseItem(itemNumber);

	item->pos.xRot = oldXrot;
	item->pos.yRot = oldYrot;
	item->pos.zRot = oldZrot;

	item->speed = LEB128::ReadInt16(m_stream);
	item->fallspeed = LEB128::ReadInt16(m_stream);

	AddActiveItem(itemNumber);

	item->itemFlags[0] = LEB128::ReadInt16(m_stream);
	item->itemFlags[1] = LEB128::ReadInt16(m_stream);

	return true;
}

bool SaveGame::readCrossbowBolt()
{
	LEB128::ReadInt16(m_stream);

	short itemNumber = CreateItem();
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->objectNumber = ID_CROSSBOW_BOLT;
	item->pos.xPos = LEB128::ReadInt32(m_stream);
	item->pos.yPos = LEB128::ReadInt32(m_stream);
	item->pos.zPos = LEB128::ReadInt32(m_stream);
	item->pos.xRot = LEB128::ReadInt16(m_stream);
	item->pos.yRot = LEB128::ReadInt16(m_stream);
	item->pos.zRot = LEB128::ReadInt16(m_stream);
	item->roomNumber = LEB128::ReadInt16(m_stream);

	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	InitialiseItem(itemNumber);

	item->pos.xRot = oldXrot;
	item->pos.yRot = oldYrot;
	item->pos.zRot = oldZrot;

	item->speed = LEB128::ReadInt16(m_stream);
	item->fallspeed = LEB128::ReadInt16(m_stream);

	AddActiveItem(itemNumber);

	return true;
}

bool SaveGame::readFlare()
{
	LEB128::ReadInt16(m_stream);

	short itemNumber = CreateItem();
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->objectNumber = ID_FLARE_ITEM;
	item->pos.xPos = LEB128::ReadInt32(m_stream);
	item->pos.yPos = LEB128::ReadInt32(m_stream);
	item->pos.zPos = LEB128::ReadInt32(m_stream);
	item->pos.xRot = LEB128::ReadInt16(m_stream);
	item->pos.yRot = LEB128::ReadInt16(m_stream);
	item->pos.zRot = LEB128::ReadInt16(m_stream);
	item->roomNumber = LEB128::ReadInt16(m_stream);

	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	InitialiseItem(itemNumber);

	item->pos.xRot = oldXrot;
	item->pos.yRot = oldYrot;
	item->pos.zRot = oldZrot;

	item->speed = LEB128::ReadInt16(m_stream);
	item->fallspeed = LEB128::ReadInt16(m_stream);

	AddActiveItem(itemNumber);

	// Flare age
	item->data = LEB128::ReadInt32(m_stream);

	return true;
}

bool SaveGame::readTorpedo()
{
	LEB128::ReadInt16(m_stream);

	short itemNumber = CreateItem();
	ITEM_INFO* item = &g_Level.Items[itemNumber];

	item->objectNumber = ID_TORPEDO;
	item->pos.xPos = LEB128::ReadInt32(m_stream);
	item->pos.yPos = LEB128::ReadInt32(m_stream);
	item->pos.zPos = LEB128::ReadInt32(m_stream);
	item->pos.xRot = LEB128::ReadInt16(m_stream);
	item->pos.yRot = LEB128::ReadInt16(m_stream);
	item->pos.zRot = LEB128::ReadInt16(m_stream);
	item->roomNumber = LEB128::ReadInt16(m_stream);

	short oldXrot = item->pos.xRot;
	short oldYrot = item->pos.yRot;
	short oldZrot = item->pos.zRot;

	InitialiseItem(itemNumber);

	item->pos.xRot = oldXrot;
	item->pos.yRot = oldYrot;
	item->pos.zRot = oldZrot;

	item->speed = LEB128::ReadInt16(m_stream);
	item->fallspeed = LEB128::ReadInt16(m_stream);

	AddActiveItem(itemNumber);

	item->itemFlags[0] = LEB128::ReadInt16(m_stream);
	item->itemFlags[1] = LEB128::ReadInt16(m_stream);
	item->currentAnimState = LEB128::ReadInt16(m_stream);
	item->goalAnimState = LEB128::ReadInt16(m_stream);
	item->requiredAnimState = LEB128::ReadInt16(m_stream);

	return true;
}

void SaveGame::saveItemQuadInfo(int itemNumber, int arg2)
{

	//m_stream->WriteBytes(reinterpret_cast<byte*>(g_Level.Items[itemNumber].data), sizeof(QUAD_INFO));
}

void SaveGame::saveRats(int arg1, int arg2)
{
	RAT_STRUCT buffer;
	memcpy(&buffer, &Rats[arg1], sizeof(RAT_STRUCT));
	LEB128::Write(m_stream, arg1);
	m_stream->Write(reinterpret_cast<char*>(&buffer), sizeof(RAT_STRUCT));
}

void SaveGame::saveBats(int arg1, int arg2)
{
	BAT_STRUCT buffer;
	memcpy(&buffer, &Bats[arg1], sizeof(BAT_STRUCT));
	LEB128::Write(m_stream, arg1);
	m_stream->Write(reinterpret_cast<char*>(&buffer), sizeof(BAT_STRUCT));
}

void SaveGame::saveSpiders(int arg1, int arg2)
{
	SPIDER_STRUCT buffer;
	memcpy(&buffer, &Spiders[arg1], sizeof(SPIDER_STRUCT));
	LEB128::Write(m_stream, arg1);
	m_stream->Write(reinterpret_cast<char*>(&buffer), sizeof(SPIDER_STRUCT));
}

bool SaveGame::readBats()
{
	int index = LEB128::ReadInt16(m_stream);

	BAT_STRUCT* bats = &Bats[index];

	char* buffer = (char*)malloc(sizeof(BAT_STRUCT));
	m_stream->Read(buffer, sizeof(BAT_STRUCT));
	memcpy(bats, buffer, sizeof(BAT_STRUCT));
	free(buffer);

	return true;
}

bool SaveGame::readRats()
{
	int index = LEB128::ReadInt16(m_stream);

	RAT_STRUCT* rats = &Rats[index];

	char* buffer = (char*)malloc(sizeof(RAT_STRUCT));
	m_stream->Read(buffer, sizeof(RAT_STRUCT));
	memcpy(rats, buffer, sizeof(RAT_STRUCT));
	free(buffer);

	return true;
}

bool SaveGame::readSpiders()
{
	int index = LEB128::ReadInt16(m_stream);

	SPIDER_STRUCT* spiders = &Spiders[index];

	char* buffer = (char*)malloc(sizeof(SPIDER_STRUCT));
	m_stream->Read(buffer, sizeof(SPIDER_STRUCT));
	memcpy(spiders, buffer, sizeof(SPIDER_STRUCT));
	free(buffer);

	return true;
}