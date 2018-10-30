#include "roomload.h"
#include "..\Global\global.h"
#include "..\Game\items.h"
#include "..\Specific\setup.h"
#include "..\Game\draw.h"
#include "..\Game\lot.h"
#include "..\Game\savegame.h"
#include "..\Scripting\GameFlowScript.h"

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
__int16* RawMeshData;
__int32 MeshDataSize;
__int32* MeshTrees;
__int32* RawMeshPointers;
__int32 NumObjects;
__int32 NumStaticObjects;
__int32 NumMeshPointers;
__int32 NumObjectTextures;

uintptr_t hLoadLevel;
unsigned __int32 ThreadId;
__int32 IsLevelLoading;

using namespace std;

vector<__int32> MoveablesIds;
vector<__int32> StaticObjectsIds;

extern GameFlow* g_GameFlow;
extern __int16 LaraVehicle;
char* LevelDataPtr;

ChunkReader* chunkIO;

__int16 ReadInt16()
{
	__int16 value = *(__int16*)LevelDataPtr;
	LevelDataPtr += 2;
	return value;
}

__int32 ReadInt32()
{
	__int32 value = *(__int32*)LevelDataPtr;
	LevelDataPtr += 4;
	return value;
}

void ReadBytes(void* dest, __int32 count)
{
	memcpy(dest, LevelDataPtr, count);
	LevelDataPtr += count;
}

__int32 __cdecl LoadItems()
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
		for (__int32 i = 0; i < NumItems; i++)
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

		for (__int32 i = 0; i < NumItems; i++)
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

void __cdecl LoadObjects()
{
	DB_Log(2, "LoadObjects - DLL");
	 
	memset(Objects, 0, sizeof(OBJECT_INFO) * NUM_OBJECTS);
	memset(StaticObjects, 0, sizeof(STATIC_INFO) * NUM_STATICS);

	__int32 numMeshDataWords = ReadInt32();
	__int32 numMeshDataBytes = 2 * numMeshDataWords;

	MeshBase = (__int16*)GameMalloc(numMeshDataBytes);
	ReadBytes(MeshBase, numMeshDataBytes);
	RawMeshData = (__int16*)malloc(numMeshDataBytes);
	memcpy(RawMeshData, MeshBase, numMeshDataBytes);

	MeshDataSize = numMeshDataBytes;

	// TR5 functions do something strange with meshes so I save just for me raw meshes and raw mesh pointers
	__int32 numMeshPointers = ReadInt32();
	Meshes = (__int16**)GameMalloc(8 * numMeshPointers);
	RawMeshPointers = (__int32*)malloc(4 * numMeshPointers);
	ReadBytes(RawMeshPointers, 4 * numMeshPointers);
	memcpy(Meshes, RawMeshPointers, 4 * numMeshPointers);

	for (__int32 i = 0; i < numMeshPointers; i++)
		Meshes[i] = &MeshBase[(__int32)Meshes[i] / 2];

	__int32 numMeshes = numMeshPointers;
	NumMeshPointers = numMeshes;

	__int32 numAnimations = ReadInt32();
	Anims = (ANIM_STRUCT*)GameMalloc(sizeof(ANIM_STRUCT) * numAnimations);
	ReadBytes(Anims, sizeof(ANIM_STRUCT) * numAnimations);

	__int32 numChanges = ReadInt32();
	Changes = (CHANGE_STRUCT*)GameMalloc(sizeof(CHANGE_STRUCT) * numChanges);
	ReadBytes(Changes, sizeof(CHANGE_STRUCT) * numChanges);

	__int32 numRanges = ReadInt32();
	Ranges = (RANGE_STRUCT*)GameMalloc(sizeof(RANGE_STRUCT) * numRanges);
	ReadBytes(Ranges, sizeof(RANGE_STRUCT) * numRanges);

	__int32 numCommands = ReadInt32();
	Commands = (__int16*)GameMalloc(2 * numCommands);
	ReadBytes(Commands, 2 * numCommands);

	__int32 numBones = ReadInt32();
	Bones = (__int32*)GameMalloc(4 * numBones);
	ReadBytes(Bones, 4 * numBones);

	__int32* bone = Bones;
	for (int i = 0; i < 15; i++)
	{
		__int32 opcode = *(bone++);
		int linkX = *(bone++);
		int linkY = *(bone++);
		int linkZ = *(bone++);
	}
	
	MeshTrees = (__int32*)GameMalloc(4 * numBones);

	memcpy(MeshTrees, Bones, 4 * numBones);

	__int32 numFrames = ReadInt32();
	Frames = (__int16*)GameMalloc(2 * numFrames);
	ReadBytes(Frames, 2 * numFrames);

	AnimationsCount = numAnimations;
	if (AnimationsCount > 0)
	{
		__int32 i = 0;
		for (__int32 i = 0; i < AnimationsCount; i++)
			ADD_PTR(Anims[i].framePtr, __int16, Frames);
	}

	__int32 numModels = ReadInt32();
	NumObjects = numModels;
	for (__int32 i = 0; i < numModels; i++)
	{
		int objNum = ReadInt32();
		MoveablesIds.push_back(objNum);

		Objects[objNum].loaded = true;
		Objects[objNum].nmeshes = ReadInt16();
		Objects[objNum].meshIndex = ReadInt16();
		Objects[objNum].boneIndex = ReadInt32();
		Objects[objNum].frameBase = (__int16*)(ReadInt32() + (__int32)Frames); 
		Objects[objNum].animIndex = ReadInt16();

		ReadInt16();

		Objects[objNum].loaded = true;
	}

	if (LaraDrawType != LARA_DIVESUIT)
		CreateSkinningData();

	for (__int32 i = 0; i < NUM_OBJECTS; i++)
	{
		Objects[i].meshIndex *= 2;
	}

	memcpy(&Meshes[numMeshes], &Meshes[0], sizeof(short*) * numMeshes);

	for (__int32 i = 0; i < numMeshes; i++)
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
		__int32 meshID = ReadInt32();
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

	for (__int32 i = 0; i < NUM_STATICS; i++)
	{
		StaticObjects[i].meshNumber *= 2;
	}

	ProcessMeshData(2 * numMeshes);

	// HACK: to remove after decompiling LoadSprites
	MoveablesIds.push_back(ID_DEFAULT_SPRITES);
}

void __cdecl LoadCameras()
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

void __cdecl LoadTextures()
{
	DB_Log(2, "LoadTextures - DLL");
	printf("LoadTextures\n");

	__int32 uncompressedSize = 0;
	__int32 compressedSize = 0;

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

void __cdecl ReadRoom(ROOM_INFO* room, ROOM_INFO* roomData)
{
	/*ADD_PTR(roomData->door, __int16, roomData + 1);
	ADD_PTR(roomData->floor, FLOOR_INFO, roomData + 1);
	ADD_PTR(roomData->light, LIGHTINFO, roomData + 1);
	ADD_PTR(roomData->mesh, MESH_INFO, roomData + 1);
	ADD_PTR(roomData->Separator4, void, roomData + 1);
	ADD_PTR(roomData->LayerOffset, tr5_room_layer, roomData + 1);
	ADD_PTR(roomData->PolyOffset, void, roomData + 1);
	ADD_PTR(roomData->PolyOffset2, void, roomData + 1);
	ADD_PTR(roomData->VerticesOffset, tr5_room_vertex, roomData + 1);

	roomData->LightDataSize += (__int32)(roomData + 1);

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

void __cdecl ReadRooms()
{
	ReadInt32();

	__int32 numRooms = ReadInt32();
	NumberRooms = numRooms;
	Rooms = (ROOM_INFO*)GameMalloc(NumberRooms * sizeof(ROOM_INFO));

	printf("NumRooms: %d\n", numRooms);
	
	for (__int32 i = 0; i < NumberRooms; i++)
	{
		// Ignore XELA
		__int32 xela = ReadInt32();

		// Read room data
		__int32 roomDataSize = ReadInt32();
		byte* roomData = (byte*)GameMalloc(roomDataSize);
		ReadBytes(roomData, roomDataSize);

		// Put the room data in the struct
		ReadRoom(&Rooms[i], (ROOM_INFO*)roomData);
	}
}
 
__int32 __cdecl LoadRoomsNew()
{
	DB_Log(2, "LoadRooms - DLL");
	printf("LoadRooms\n");
	
	Wibble = 0;
	RoomLightsCount = 0;
	Unk_007E7FE8 = 0;

	ReadRooms();
	DoSomethingWithRooms();

	__int32 numFloorData = ReadInt32(); 
	FloorData = (__int16*)GameMalloc(numFloorData * 2);
	ReadBytes(FloorData, numFloorData * 2);

	return true;
}

void __cdecl FreeLevel()
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
		__int32 version;
		__int16 numRoomTextureTiles;
		__int16 numObjectsTextureTiles;
		__int16 numBumpTextureTiles;

		ReadFileEx(&version, 1, 4, LevelFilePtr);
		ReadFileEx(&numRoomTextureTiles, 1, 2, LevelFilePtr);
		ReadFileEx(&numObjectsTextureTiles, 1, 2, LevelFilePtr);
		ReadFileEx(&numBumpTextureTiles, 1, 2, LevelFilePtr);

		g_Renderer->NumTexturePages = numRoomTextureTiles + numObjectsTextureTiles + numBumpTextureTiles;

		LoadTextures();

		g_Renderer->UpdateProgress(20);

		__int16 buffer[32];
		ReadFileEx(&buffer, 2, 16, LevelFilePtr);
		WeatherType = buffer[0];
		LaraDrawType = buffer[1];

		__int32 uncompressedSize;
		__int32 compressedSize;

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

		__int32 extraSize = 0;
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

	g_Renderer->UpdateProgress(80);
	 
	g_Renderer->PrepareDataForTheRenderer();
	
	// Initialise the game
	GameScriptLevel* level = g_GameFlow->GetLevel(CurrentLevel);

	SeedRandomDraw(0xD371F947);
	SeedRandomControl(0xD371F947);
	Wibble = 0;
	TorchRoom = -1;
	InitialiseGameFlags();
	InitialiseLara(!(gfInitialiseGame || CurrentLevel == 1 || level->ResetHub));
	GetCarriedItems();
	GetAIPickups();
	SeedRandomDraw(0xD371F947);
	SeedRandomControl(0xD371F947);
	LaraVehicle = -1;
	g_GameScript->AssignItemsAndLara();

	// Level loaded
	IsLevelLoading = false;
	g_Renderer->UpdateProgress(100);

	_endthreadex(1);

	return true;
}

__int32 __cdecl S_LoadLevelFile(__int32 levelIndex)
{
	DB_Log(2, "S_LoadLevelFile - DLL");
	printf("S_LoadLevelFile\n");
	 
	SOUND_Stop();
	Sound_FreeSamples();
	FreeLevel();

	RenderLoadBar = false;
	
	char filename[80];
	GameScriptLevel* level = g_GameFlow->GetLevel(levelIndex);
	strcpy_s(filename, level->FileName.c_str());
	
	// Loading level is done is two threads, one for loading level and one for drawing loading screen
	IsLevelLoading = true;
	hLoadLevel = _beginthreadex(0, 0, LoadLevel, filename, 0, &ThreadId);

	// This function loops until progress is 100%. Not very thread safe, but behavious should be predictable.
	g_Renderer->DrawLoadingScreen((char*)(level->LoadScreenFileName.c_str()));

	return true;
}

void __cdecl AdjustUV(__int32 num)
{
	// Dummy function
	NumObjectTextures = num;
}

bool __cdecl ReadLuaIds(ChunkId* chunkId, __int32 maxSize, __int32 arg)
{
	if (chunkId->EqualsTo(ChunkLuaId))
	{
		__int32 luaId = 0;
		chunkIO->GetRawStream()->ReadInt32(&luaId);
		
		__int32 itemId = 0;
		chunkIO->GetRawStream()->ReadInt32(&itemId);

		g_GameScript->AddLuaId(luaId, itemId);

		return true;
	}
	else
		return false;
}

bool __cdecl ReadLuaTriggers(ChunkId* chunkId, __int32 maxSize, __int32 arg)
{
	if (chunkId->EqualsTo(ChunkTrigger))
	{
		char* functionName = NULL;
		chunkIO->GetRawStream()->ReadString(&functionName);
		
		char* functionCode = NULL;
		chunkIO->GetRawStream()->ReadString(&functionCode);

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

bool __cdecl ReadNewDataChunks(ChunkId* chunkId, __int32 maxSize, __int32 arg)
{
	if (chunkId->EqualsTo(ChunkTriggersList))
		return chunkIO->ReadChunks(ReadLuaTriggers, 0);
	else if (chunkId->EqualsTo(ChunkLuaIds))
		return chunkIO->ReadChunks(ReadLuaIds, 0);
	return false;
}

void __cdecl LoadNewData(__int32 size)
{
	// Free old level scripts
	MemoryStream stream(LevelDataPtr, size);
	chunkIO = new ChunkReader(0x4D355254, &stream);
	if (!chunkIO->IsValid())
		return;

	chunkIO->ReadChunks(ReadNewDataChunks, 0);
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