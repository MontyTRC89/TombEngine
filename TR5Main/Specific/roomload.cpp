#include "roomload.h"
#include "..\Global\global.h"
#include "..\Game\items.h"
#include "..\Specific\setup.h"
#include "..\Game\draw.h"
#include "..\Game\lot.h"
#include "..\Game\Lara.h"
#include "..\Game\savegame.h"
#include "..\Game\spotcam.h"
#include "..\Scripting\GameFlowScript.h"
#include "..\Game\control.h"
#include "..\Game\pickup.h"

#include "IO/ChunkId.h"
#include "IO/ChunkReader.h"
#include "IO/ChunkWriter.h" 
#include "IO/LEB128.h"

#include <process.h>
#include <stdio.h>
#include <vector>
#include <map>
#include "IO/Streams.h"

ChunkId* ChunkTriggersList = ChunkId::FromString("Tr5Triggers");
ChunkId* ChunkTrigger = ChunkId::FromString("Tr5Trigger");
ChunkId* ChunkLuaIds = ChunkId::FromString("Tr5LuaIds");
ChunkId* ChunkLuaId = ChunkId::FromString("Tr5LuaId");

extern GameScript* g_GameScript;

byte* Texture32;
byte* Texture16;
byte* MiscTextures;
short* RawMeshData;
int MeshDataSize;
int* MeshTrees;
int* RawMeshPointers;
int NumObjects;
int NumStaticObjects;
int NumMeshPointers;
int NumObjectTextures;
int NumTextureTiles;

uintptr_t hLoadLevel;
unsigned int ThreadId;
int IsLevelLoading;
bool g_FirstLevel = true;

using namespace std;

vector<int> MoveablesIds;
vector<int> StaticObjectsIds;

extern GameFlow* g_GameFlow;
extern LaraExtraInfo g_LaraExtra;
char* LevelDataPtr;

int g_NumSprites;
int g_NumSpritesSequences;

ChunkReader* g_levelChunkIO;

short ReadInt8()
{
	byte value = *(byte*)LevelDataPtr;
	LevelDataPtr += 1;
	return value;
}

short ReadInt16()
{
	short value = *(short*)LevelDataPtr;
	LevelDataPtr += 2;
	return value;
}

int ReadInt32()
{
	int value = *(int*)LevelDataPtr;
	LevelDataPtr += 4;
	return value;
}

void ReadBytes(void* dest, int count)
{
	memcpy(dest, LevelDataPtr, count);
	LevelDataPtr += count;
}

int LoadItems()
{
	DB_Log(2, "LoadItems - DLL");

	NumItems = ReadInt32();
	if (NumItems == 0)
		return false;

	Items = (ITEM_INFO*)GameMalloc(sizeof(ITEM_INFO) * NUM_ITEMS);
	LevelItems = NumItems;

	InitialiseClosedDoors();
	FreeItemsStuff(NUM_ITEMS);

	if (NumItems > 0)
	{
		for (int i = 0; i < NumItems; i++)
		{
			ITEM_INFO* item = &Items[i];
			
			item->objectNumber = ReadInt16();
			item->roomNumber = ReadInt16();
			item->pos.xPos = ReadInt32();
			item->pos.yPos = ReadInt32();
			item->pos.zPos = ReadInt32();
			item->pos.yRot = ReadInt16();
			item->shade = ReadInt16();
			item->triggerFlags = ReadInt16();
			item->flags = ReadInt16();

			// DEBUG: set Lara position
			/*if (item->objectNumber == ID_LARA)
			{
				item->pos.xPos = 33050;
				item->pos.yPos = 10000;
				item->pos.zPos = 21162;
				item->roomNumber = 0;
			}*/

			// ANDREA2
			/*if (item->objectNumber == ID_LARA)
			{
				item->pos.xPos = 58*1024;
				item->pos.yPos = 0;
				item->pos.zPos = 39*1024;
				item->roomNumber = 144;
			}*/
		}

		for (int i = 0; i < NumItems; i++)
			InitialiseItem(i);
	}

	for (int r = 0; r < NumberRooms; r++)
	{
		MESH_INFO* mesh = Rooms[r].mesh;

		for (int m = 0; m < Rooms[r].numMeshes; m++)
		{
			FLOOR_INFO* floor = &Rooms[r].floor[((mesh->z - Rooms[r].z) >> 10) + Rooms[r].xSize * ((mesh->x - Rooms[r].x) >> 10)];
			 
			if (!(Boxes[floor->box].overlapIndex & 0x4000)
				&& !(CurrentLevel == 5 && (r == 19 || r == 23 || r == 16)))
			{
				int fl = floor->floor << 2;
				STATIC_INFO* st = &StaticObjects[mesh->staticNumber];
				if (fl <= mesh->y - st->yMaxc + 512 && fl < mesh->y - st->yMinc)
				{
					if (st->xMaxc == 0 || st->xMinc == 0 ||
						st->zMaxc == 0 || st->zMinc == 0 ||
						(st->xMaxc ^ st->xMinc) & 0x8000 &&
						(st->xMaxc ^ st->zMinc) & 0x8000)
					{
						floor->box |= 8; 
					}
				}
			}
		}
	}

	return true;
}

void LoadObjects()
{
	DB_Log(2, "LoadObjects - DLL");
	 
	memset(Objects, 0, sizeof(OBJECT_INFO) * ID_NUMBER_OBJECTS);
	memset(StaticObjects, 0, sizeof(STATIC_INFO) * NUM_STATICS);

	int numMeshDataWords = ReadInt32();
	int numMeshDataBytes = 2 * numMeshDataWords;

	MeshBase = (short*)GameMalloc(numMeshDataBytes);
	ReadBytes(MeshBase, numMeshDataBytes);
	RawMeshData = (short*)malloc(numMeshDataBytes);
	memcpy(RawMeshData, MeshBase, numMeshDataBytes);

	MeshDataSize = numMeshDataBytes;

	// TR5 functions do something strange with meshes so I save just for me raw meshes and raw mesh pointers
	int numMeshPointers = ReadInt32();
	Meshes = (short**)GameMalloc(8 * numMeshPointers);
	RawMeshPointers = (int*)malloc(4 * numMeshPointers);
	ReadBytes(RawMeshPointers, 4 * numMeshPointers);
	memcpy(Meshes, RawMeshPointers, 4 * numMeshPointers);

	for (int i = 0; i < numMeshPointers; i++)
		Meshes[i] = &MeshBase[(int)Meshes[i] / 2];

	int numMeshes = numMeshPointers;
	NumMeshPointers = numMeshes;

	int numAnimations = ReadInt32();
	Anims = (ANIM_STRUCT*)GameMalloc(sizeof(ANIM_STRUCT) * numAnimations);
	ReadBytes(Anims, sizeof(ANIM_STRUCT) * numAnimations);

	int numChanges = ReadInt32();
	Changes = (CHANGE_STRUCT*)GameMalloc(sizeof(CHANGE_STRUCT) * numChanges);
	ReadBytes(Changes, sizeof(CHANGE_STRUCT) * numChanges);

	int numRanges = ReadInt32();
	Ranges = (RANGE_STRUCT*)GameMalloc(sizeof(RANGE_STRUCT) * numRanges);
	ReadBytes(Ranges, sizeof(RANGE_STRUCT) * numRanges);

	int numCommands = ReadInt32();
	Commands = (short*)GameMalloc(2 * numCommands);
	ReadBytes(Commands, 2 * numCommands);

	int numBones = ReadInt32();
	Bones = (int*)GameMalloc(4 * numBones);
	ReadBytes(Bones, 4 * numBones);

	int* bone = Bones;
	for (int i = 0; i < 15; i++)
	{
		int opcode = *(bone++);
		int linkX = *(bone++);
		int linkY = *(bone++);
		int linkZ = *(bone++);
	}
	
	MeshTrees = (int*)GameMalloc(4 * numBones);

	memcpy(MeshTrees, Bones, 4 * numBones);

	int numFrames = ReadInt32();
	Frames = (short*)GameMalloc(2 * numFrames);
	ReadBytes(Frames, 2 * numFrames);

	AnimationsCount = numAnimations;
	if (AnimationsCount > 0)
	{
		int i = 0;
		for (int i = 0; i < AnimationsCount; i++)
			ADD_PTR(Anims[i].framePtr, short, Frames);
	}

	int numModels = ReadInt32();
	NumObjects = numModels;
	for (int i = 0; i < numModels; i++)
	{
		int objNum = ReadInt32();
		MoveablesIds.push_back(objNum);

		Objects[objNum].loaded = true;
		Objects[objNum].nmeshes = ReadInt16();
		Objects[objNum].meshIndex = ReadInt16();
		Objects[objNum].boneIndex = ReadInt32();
		Objects[objNum].frameBase = (short*)(ReadInt32() + (int)Frames); 
		Objects[objNum].animIndex = ReadInt16();

		ReadInt16();

		Objects[objNum].loaded = true;
	}

	// TODO: this functions seems to not be useful anymore. Hairs and skinning works fine without it.
	//if (LaraDrawType != LARA_DIVESUIT)
	//	CreateSkinningData();

	for (int i = 0; i < ID_NUMBER_OBJECTS; i++)
	{
		Objects[i].meshIndex *= 2;
	}

	memcpy(&Meshes[numMeshes], &Meshes[0], sizeof(short*) * numMeshes);

	for (int i = 0; i < numMeshes; i++)
	{
		Meshes[2 * i] = Meshes[numMeshes + i];
		Meshes[2 * i + 1] = Meshes[numMeshes + i];
	}

	InitialiseObjects();
	InitialiseClosedDoors();

	int numStatics = ReadInt32();
	NumStaticObjects = numStatics;
	for (int i = 0; i < numStatics; i++)
	{
		int meshID = ReadInt32();
		StaticObjectsIds.push_back(meshID);

		StaticObjects[meshID].meshNumber = ReadInt16();

		StaticObjects[meshID].yMinp = ReadInt16();
		StaticObjects[meshID].xMaxp = ReadInt16();
		StaticObjects[meshID].yMinp = ReadInt16();
		StaticObjects[meshID].yMaxp = ReadInt16();
		StaticObjects[meshID].zMinp = ReadInt16();
		StaticObjects[meshID].zMaxp = ReadInt16();

		StaticObjects[meshID].xMinc = ReadInt16();
		StaticObjects[meshID].xMaxc = ReadInt16();
		StaticObjects[meshID].yMinc = ReadInt16();
		StaticObjects[meshID].yMaxc = ReadInt16();
		StaticObjects[meshID].zMinc = ReadInt16();
		StaticObjects[meshID].zMaxc = ReadInt16();

		StaticObjects[meshID].flags = ReadInt16();
	}

	for (int i = 0; i < NUM_STATICS; i++)
	{
		StaticObjects[i].meshNumber *= 2;
	}

	ProcessMeshData(2 * numMeshes);

	// HACK: to remove after decompiling LoadSprites
	MoveablesIds.push_back(ID_DEFAULT_SPRITES);
}

void LoadCameras()
{
	NumberCameras = ReadInt32();
	if (NumberCameras != 0)
	{
		Cameras = (OBJECT_VECTOR*)GameMalloc(NumberCameras * sizeof(OBJECT_VECTOR));
		ReadBytes(Cameras, NumberCameras * sizeof(OBJECT_VECTOR));
	}

	NumberSpotcams = ReadInt32();

	if (NumberSpotcams != 0)
	{
		ReadBytes(SpotCam, NumberSpotcams * sizeof(SPOTCAM));
	}
}

void LoadTextures()
{
	DB_Log(2, "LoadTextures - DLL");
	printf("LoadTextures\n");

	int uncompressedSize = 0;
	int compressedSize = 0;

	// Read 32 bit textures
	ReadFileEx(&uncompressedSize, 1, 4, LevelFilePtr);
	ReadFileEx(&compressedSize, 1, 4, LevelFilePtr);
	
	Texture32 = (byte*)malloc(uncompressedSize);
	byte* buffer = (byte*)malloc(compressedSize);
	
	ReadFileEx(buffer, compressedSize, 1, LevelFilePtr);
	Decompress(Texture32, buffer, compressedSize, uncompressedSize);
	free(buffer);

	// Read 16 bit textures
	ReadFileEx(&uncompressedSize, 1, 4, LevelFilePtr);
	ReadFileEx(&compressedSize, 1, 4, LevelFilePtr);

	Texture16 = (byte*)malloc(uncompressedSize);
	buffer = (byte*)malloc(compressedSize);

	ReadFileEx(buffer, compressedSize, 1u, LevelFilePtr);
	Decompress(Texture16, buffer, compressedSize, uncompressedSize);
	free(buffer);

	// Read misc textures
	ReadFileEx(&uncompressedSize, 1, 4, LevelFilePtr);
	ReadFileEx(&compressedSize, 1, 4, LevelFilePtr);

	printf("%d\n", uncompressedSize);

	MiscTextures = (byte*)malloc(uncompressedSize);
	buffer = (byte*)malloc(compressedSize);

	ReadFileEx(buffer, compressedSize, 1u, LevelFilePtr);
	Decompress(MiscTextures, buffer, compressedSize, uncompressedSize);
	free(buffer);
}

void ReadRoom(ROOM_INFO* room, ROOM_INFO* roomData)
{
	/*ADD_PTR(roomData->door, short, roomData + 1);
	ADD_PTR(roomData->floor, FLOOR_INFO, roomData + 1);
	ADD_PTR(roomData->light, LIGHTINFO, roomData + 1);
	ADD_PTR(roomData->mesh, MESH_INFO, roomData + 1);
	ADD_PTR(roomData->Separator4, void, roomData + 1);
	ADD_PTR(roomData->LayerOffset, tr5_room_layer, roomData + 1);
	ADD_PTR(roomData->PolyOffset, void, roomData + 1);
	ADD_PTR(roomData->PolyOffset2, void, roomData + 1);
	ADD_PTR(roomData->VerticesOffset, tr5_room_vertex, roomData + 1);

	roomData->LightDataSize += (int)(roomData + 1);

	if ((byte)roomData->door & 1)
	{
		//DB_Log(0, "%X", roomData->door);
		roomData->door = 0;
	}

	byte* polyOff = (byte*)roomData->PolyOffset;
	byte* polyOff2 = (byte*)roomData->PolyOffset2;
	byte* vertOff = (byte*)roomData->VerticesOffset;

	for (int i = 0; i < roomData->NumLayers; i++)
	{
		roomData->LayerOffset[i].PolyOffset = polyOff;
		roomData->LayerOffset[i].PolyOffset2 = polyOff2;
		roomData->LayerOffset[i].VerticesOffset = vertOff;

		polyOff += sizeof(tr4_mesh_face3) * roomData->LayerOffset[i].NumLayerTriangles +
			sizeof(tr4_mesh_face4) * roomData->LayerOffset[i].NumLayerRectangles;

		polyOff2 += 4 * roomData->LayerOffset[i].NumLayerVertices; // todo find what struct this is

		vertOff += sizeof(tr5_room_vertex) * roomData->LayerOffset[i].NumLayerVertices;
	}

	memcpy(room, roomData, sizeof(ROOM_INFO));*/
}

void ReadRooms()
{
	ReadInt32();

	int numRooms = ReadInt32();
	NumberRooms = numRooms;
	Rooms = (ROOM_INFO*)GameMalloc(NumberRooms * sizeof(ROOM_INFO));

	printf("NumRooms: %d\n", numRooms);
	
	for (int i = 0; i < NumberRooms; i++)
	{
		// Ignore XELA
		int xela = ReadInt32();

		// Read room data
		int roomDataSize = ReadInt32();
		byte* roomData = (byte*)GameMalloc(roomDataSize);
		ReadBytes(roomData, roomDataSize);

		// Put the room data in the struct
		ReadRoom(&Rooms[i], (ROOM_INFO*)roomData);
	}
}
 
int LoadRoomsNew()
{
	DB_Log(2, "LoadRooms - DLL");
	printf("LoadRooms\n");
	
	Wibble = 0;
	RoomLightsCount = 0;
	Unk_007E7FE8 = 0;

	ReadRooms();
	DoSomethingWithRooms();

	int numFloorData = ReadInt32(); 
	FloorData = (short*)GameMalloc(numFloorData * 2);
	ReadBytes(FloorData, numFloorData * 2);

	return true;
}

void FreeLevel()
{
	MallocPtr = MallocBuffer;
	MallocFree = MallocSize;
	g_Renderer->FreeRendererData();
	g_GameScript->FreeLevelScripts();
}

unsigned __stdcall LoadLevel(void* data)
{
	DB_Log(5, "LoadLevel - DLL");
	printf("LoadLevel\n");

	char* filename = (char*)data;

	LevelDataPtr = NULL;
	LevelFilePtr = 0;

	g_Renderer->UpdateProgress(0);

	LevelFilePtr = FileOpen(filename);
	if (LevelFilePtr)
	{
		int version;
		short numRoomTextureTiles;
		short numObjectsTextureTiles;
		short numBumpTextureTiles;

		ReadFileEx(&version, 1, 4, LevelFilePtr);
		ReadFileEx(&numRoomTextureTiles, 1, 2, LevelFilePtr);
		ReadFileEx(&numObjectsTextureTiles, 1, 2, LevelFilePtr);
		ReadFileEx(&numBumpTextureTiles, 1, 2, LevelFilePtr);

		g_Renderer->NumTexturePages = numRoomTextureTiles + numObjectsTextureTiles + numBumpTextureTiles;

		LoadTextures();

		g_Renderer->UpdateProgress(20);

		short buffer[32];
		ReadFileEx(&buffer, 2, 16, LevelFilePtr);
		WeatherType = buffer[0];
		LaraDrawType = buffer[1];

		int uncompressedSize;
		int compressedSize;

		ReadFileEx(&uncompressedSize, 1, 4, LevelFilePtr);
		ReadFileEx(&compressedSize, 1, 4, LevelFilePtr);

		LevelDataPtr = (char*)malloc(uncompressedSize);
		ReadFileEx(LevelDataPtr, uncompressedSize, 1, LevelFilePtr);

		LoadRooms();
		g_Renderer->UpdateProgress(40);

		LoadObjects();
		g_Renderer->UpdateProgress(50);

		InitialiseLOTarray(true);
		LoadSprites();
		LoadCameras();
		LoadSoundEffects();
		g_Renderer->UpdateProgress(60);

		LoadBoxes();
		LoadAnimatedTextures();
		LoadTextureInfos();
		g_Renderer->UpdateProgress(70);

		LoadItems();
		LoadAIObjects();
		LoadDemoData();
		LoadSamples();
		g_Renderer->UpdateProgress(80);

		int extraSize = 0;
		ReadFileEx(&extraSize, 1, 4, LevelFilePtr);
		if (extraSize > 0)
		{
			ReadFileEx(&extraSize, 1, 4, LevelFilePtr);
			LevelDataPtr = (char*)malloc(extraSize);
			ReadFileEx(LevelDataPtr, extraSize, 1, LevelFilePtr);

			LoadNewData(extraSize);
		}

		LevelDataPtr = NULL;
		FileClose(LevelFilePtr);
	}
	else
	{
		return false;
	}

	g_Renderer->UpdateProgress(90);
	g_Renderer->PrepareDataForTheRenderer();
	
	// Initialise the game
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	SeedRandomDraw(0xD371F947);
	SeedRandomControl(0xD371F947);
	Wibble = 0;
	TorchRoom = -1;
	InitialiseGameFlags();
	InitialiseLara(!(InitialiseGame || CurrentLevel == 1 || level->ResetHub));
	GetCarriedItems();
	GetAIPickups();
	SeedRandomDraw(0xD371F947);
	SeedRandomControl(0xD371F947);
	g_LaraExtra.Vehicle = -1;
	g_GameScript->AssignItemsAndLara();

	// Level loaded
	IsLevelLoading = false;
	g_Renderer->UpdateProgress(100);

	_endthreadex(1);

	return true;
}

int S_LoadLevelFile(int levelIndex)
{
	DB_Log(2, "S_LoadLevelFile - DLL");
	printf("S_LoadLevelFile\n");
	 
	SOUND_Stop();
	Sound_FreeSamples();
	if (!g_FirstLevel)
		FreeLevel();
	g_FirstLevel = false;
	RenderLoadBar = false;
	
	char filename[80];
	GameScriptLevel* level = g_GameFlow->Levels[levelIndex];
	strcpy_s(filename, level->FileName.c_str());
	
	// Loading level is done is two threads, one for loading level and one for drawing loading screen
	IsLevelLoading = true;
	hLoadLevel = _beginthreadex(0, 0, LoadLevel, filename, 0, &ThreadId);

	// This function loops until progress is 100%. Not very thread safe, but behaviour should be predictable.
	g_Renderer->DrawLoadingScreen((char*)(level->LoadScreenFileName.c_str()));

	while (IsLevelLoading);

	return true;
}

void AdjustUV(int num)
{
	// Dummy function
	NumObjectTextures = num;
}

bool ReadLuaIds(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(ChunkLuaId))
	{
		int luaId = 0;
		g_levelChunkIO->GetRawStream()->ReadInt32(&luaId);
		
		int itemId = 0;
		g_levelChunkIO->GetRawStream()->ReadInt32(&itemId);

		g_GameScript->AddLuaId(luaId, itemId);

		return true;
	}
	else
		return false;
}

bool ReadLuaTriggers(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(ChunkTrigger))
	{
		char* functionName = NULL;
		g_levelChunkIO->GetRawStream()->ReadString(&functionName);
		
		char* functionCode = NULL;
		g_levelChunkIO->GetRawStream()->ReadString(&functionCode);

		LuaFunction* function = new LuaFunction();
		function->Name = string(functionName);
		function->Code = string(functionCode);
		function->Executed = false;

		g_GameScript->AddTrigger(function);

		delete functionName;
		delete functionCode;

		return true;
	}
	else
		return false;
}

bool ReadNewDataChunks(ChunkId* chunkId, int maxSize, int arg)
{
	if (chunkId->EqualsTo(ChunkTriggersList))
		return g_levelChunkIO->ReadChunks(ReadLuaTriggers, 0);
	else if (chunkId->EqualsTo(ChunkLuaIds))
		return g_levelChunkIO->ReadChunks(ReadLuaIds, 0);
	return false;
}

void LoadNewData(int size)
{
	// Free old level scripts
	MemoryStream stream(LevelDataPtr, size);
	g_levelChunkIO = new ChunkReader(0x4D355254, &stream);
	if (!g_levelChunkIO->IsValid())
		return;

	g_levelChunkIO->ReadChunks(ReadNewDataChunks, 0);
}

void LoadSprites()
{
	DB_Log(2, "LoadSprites");

	ReadInt32(); // SPR\0

	g_NumSprites = ReadInt32();

	Sprites = (SPRITE*)GameMalloc(g_NumSprites * sizeof(SPRITE));

	for (int i = 0; i < g_NumSprites; i++)
	{
		Sprites[i].tile = ReadInt16() + 1;
		Sprites[i].x = ReadInt8();
		Sprites[i].y = ReadInt8();
		Sprites[i].width = ReadInt16();
		Sprites[i].height = ReadInt16();
		Sprites[i].left = (ReadInt16() + 1) / 256.0;
		Sprites[i].top = (ReadInt16() + 1) / 256.0;
		Sprites[i].right = (ReadInt16() - 1) / 256.0;
		Sprites[i].bottom = (ReadInt16() - 1) / 256.0;
	}

	g_NumSpritesSequences = ReadInt32();

	for (int i = 0; i < g_NumSpritesSequences; i++)
	{
		int spriteID = ReadInt32();
		short negLength = ReadInt16();
		short offset = ReadInt16();
		if (spriteID >= ID_NUMBER_OBJECTS)
		{
			StaticObjects[spriteID - ID_NUMBER_OBJECTS].meshNumber = offset;
		}
		else
		{
			Objects[spriteID].nmeshes = negLength;
			Objects[spriteID].meshIndex = offset;
			Objects[spriteID].loaded = true;
		}
	}
}

void GetCarriedItems()
{
	int i;
	ITEM_INFO* item, *item2;
	short linknum;

	for (i = 0; i < LevelItems; ++i)
		Items[i].carriedItem = NO_ITEM;
	for (i = 0; i < LevelItems; ++i)
	{
		item = &Items[i];
		if (Objects[item->objectNumber].intelligent || item->objectNumber >= ID_SEARCH_OBJECT1 && item->objectNumber <= ID_SEARCH_OBJECT3)
		{
			for (linknum = Rooms[item->roomNumber].itemNumber; linknum != NO_ITEM; linknum = Items[linknum].nextItem)
			{
				item2 = &Items[linknum];
				if (abs(item2->pos.xPos - item->pos.xPos) < 512
					&& abs(item2->pos.zPos - item->pos.zPos) < 512
					&& abs(item2->pos.yPos - item->pos.yPos) < 256
					&& Objects[item2->objectNumber].collision == PickupCollision)
				{
					item2->carriedItem = item->carriedItem;
					item->carriedItem = linknum;
					RemoveDrawnItem(linknum);
					item2->roomNumber = NO_ROOM;
				}
			}
		}
	}
}

void GetAIPickups()
{
	int i, num;
	ITEM_INFO* item;
	AIOBJECT* object;

	for (i = 0; i < LevelItems; ++i)
	{
		item = &Items[i];
		if (Objects[item->objectNumber].intelligent)
		{
			item->aiBits = 0;
			for (num = 0; num < nAIObjects; ++num)
			{
				object = &AIObjects[num];
				if (abs(object->x - item->pos.xPos) < 512
					&& abs(object->z - item->pos.zPos) < 512
					&& object->roomNumber == item->roomNumber
					&& object->objectNumber < ID_AI_PATROL2)
				{
					item->aiBits = (1 << object->objectNumber - ID_AI_GUARD) & 0x1F;
					item->itemFlags[3] = object->triggerFlags;
					if (object->objectNumber != ID_AI_GUARD)
						object->roomNumber = NO_ROOM;
				}
			}
			item->TOSSPAD |= item->aiBits << 8 | (char) item->itemFlags[3];
		}
	}
}

void Inject_RoomLoad()
{
	INJECT(0x004A6380, LoadItems);
	INJECT(0x004A4E60, LoadObjects);
	INJECT(0x004A3FC0, LoadTextures);
	INJECT(0x0040130C, S_LoadLevelFile);
	INJECT(0x004A7130, FreeLevel);
	INJECT(0x004A5430, AdjustUV);
	INJECT(0x004A5CA0, LoadCameras);
}