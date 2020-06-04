#include "framework.h"
#include "level.h"
#include "setup.h"
#include "draw.h"
#include "lot.h"
#include "Lara.h"
#include "savegame.h"
#include "spotcam.h"
#include "camera.h"
#include "control.h"
#include "pickup.h"
#include "door.h"
#include "box.h"
#include "sound.h"
#include "levelloader.h"
#include "GameFlowScript.h"

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
short* MeshBase;
short** Meshes;
int NumItems;
FILE* LevelFilePtr;

uintptr_t hLoadLevel;
unsigned int ThreadId;
int IsLevelLoading;
bool g_FirstLevel = true;

vector<int> MoveablesIds;
vector<int> StaticObjectsIds;

extern GameFlow* g_GameFlow;

char* LevelDataPtr;
OBJECT_TEXTURE* ObjectTextures;
ITEM_INFO* Items;
int LevelItems;
int NumberRooms;
ROOM_INFO* Rooms;
ANIM_STRUCT* Anims;
CHANGE_STRUCT* Changes;
RANGE_STRUCT* Ranges;
short* Commands;
int* Bones;
short* Frames;
int AnimationsCount;
short* FloorData;
int nAIObjects;
AIOBJECT* AIObjects;
SPRITE* Sprites;

int g_NumSprites;
int g_NumSpritesSequences;

ChunkReader* g_levelChunkIO;

TrLevel g_Level;

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
	NumItems = ReadInt32();
	if (NumItems == 0)
		return false;

	Items = (ITEM_INFO*)game_malloc(sizeof(ITEM_INFO) * NUM_ITEMS);
	LevelItems = NumItems;

	InitialiseClosedDoors();
	InitialiseItemArray(NUM_ITEMS);

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
				StaticInfo* st = &StaticObjects[mesh->staticNumber];
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
	memset(Objects, 0, sizeof(ObjectInfo) * ID_NUMBER_OBJECTS);
	memset(StaticObjects, 0, sizeof(StaticInfo) * MAX_STATICS);

	int numMeshDataWords = ReadInt32();
	int numMeshDataBytes = 2 * numMeshDataWords;

	MeshBase = (short*)game_malloc(numMeshDataBytes);
	ReadBytes(MeshBase, numMeshDataBytes);

	MeshDataSize = numMeshDataBytes;

	int numMeshPointers = ReadInt32();
	Meshes = (short**)game_malloc(sizeof(short*) * numMeshPointers);
	ReadBytes(Meshes, sizeof(short*) * numMeshPointers);

	for (int i = 0; i < numMeshPointers; i++) 
	{
		Meshes[i] = &MeshBase[(int)Meshes[i] / 2];
	}

	int numMeshes = numMeshPointers;
	NumMeshPointers = numMeshes;

	int numAnimations = ReadInt32();
	Anims = (ANIM_STRUCT*)game_malloc(sizeof(ANIM_STRUCT) * numAnimations);
	ReadBytes(Anims, sizeof(ANIM_STRUCT) * numAnimations);

	int numChanges = ReadInt32();
	Changes = (CHANGE_STRUCT*)game_malloc(sizeof(CHANGE_STRUCT) * numChanges);
	ReadBytes(Changes, sizeof(CHANGE_STRUCT) * numChanges);

	int numRanges = ReadInt32();
	Ranges = (RANGE_STRUCT*)game_malloc(sizeof(RANGE_STRUCT) * numRanges);
	ReadBytes(Ranges, sizeof(RANGE_STRUCT) * numRanges);

	int numCommands = ReadInt32();
	Commands = (short*)game_malloc(sizeof(short) * numCommands);
	ReadBytes(Commands, sizeof(short) * numCommands);

	int numBones = ReadInt32();
	Bones = (int*)game_malloc(sizeof(int) * numBones);
	ReadBytes(Bones, sizeof(int) * numBones);

	int* bone = Bones;
	for (int i = 0; i < 15; i++)
	{
		int opcode = *(bone++);
		int linkX = *(bone++);
		int linkY = *(bone++);
		int linkZ = *(bone++);
	}
	
	MeshTrees = (int*)game_malloc(sizeof(int) * numBones);

	memcpy(MeshTrees, Bones, sizeof(int) * numBones);

	int numFrames = ReadInt32();
	Frames = (short*)game_malloc(sizeof(short) * numFrames);
	ReadBytes(Frames, sizeof(short) * numFrames);

	AnimationsCount = numAnimations;
	if (AnimationsCount > 0)
	{
		int i = 0;
		for (int i = 0; i < AnimationsCount; i++)
			AddPtr(Anims[i].framePtr, short, Frames);
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

	// HACK: to remove after decompiling LoadSprites
	MoveablesIds.push_back(ID_DEFAULT_SPRITES);
}

void LoadCameras()
{
	NumberCameras = ReadInt32();
	if (NumberCameras != 0)
	{
		Camera.fixed = (OBJECT_VECTOR*)game_malloc(NumberCameras * sizeof(OBJECT_VECTOR));
		ReadBytes(Camera.fixed, NumberCameras * sizeof(OBJECT_VECTOR));
	}

	NumberSpotcams = ReadInt32();

	if (NumberSpotcams != 0)
	{
		ReadBytes(SpotCam, NumberSpotcams * sizeof(SPOTCAM));
	}
}

void LoadTextures()
{
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
		////DB_Log(0, "%X", roomData->door);
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

void FixUpRoom(ROOM_INFO* room, ROOM_INFO* roomData) 
{
	AddPtr(roomData->door, short, roomData + 1);
	AddPtr(roomData->floor, FLOOR_INFO, roomData + 1);
	AddPtr(roomData->light, tr5_room_light, roomData + 1);
	AddPtr(roomData->mesh, MESH_INFO, roomData + 1);
	//AddPtr(roomData->RoomLights, tr5_room_light, roomData + 1);
	AddPtr(roomData->LayerOffset, tr5_room_layer, roomData + 1);
	AddPtr(roomData->PolyOffset, void, roomData + 1);
	AddPtr(roomData->PolyOffset2, void, roomData + 1);
	AddPtr(roomData->VerticesOffset, tr5_room_vertex, roomData + 1);

	roomData->LightDataSize += (uint32_t)(roomData + 1);

	if ((uint8_t)roomData->door & 1)
	{
		roomData->door = nullptr;
	}

	auto* polyOff = (char*)roomData->PolyOffset;
	auto* polyOff2 = (char*)roomData->PolyOffset2;
	auto* vertOff = (char*)roomData->VerticesOffset;

	for (int i = 0; i < roomData->NumLayers; i++)
	{
		roomData->LayerOffset[i].PolyOffset = polyOff;
		roomData->LayerOffset[i].PolyOffset2 = polyOff2;
		roomData->LayerOffset[i].VerticesOffset = vertOff;

		polyOff += sizeof(struct tr4_mesh_face3) * roomData->LayerOffset[i].NumLayerTriangles +
			sizeof(struct tr4_mesh_face4) * roomData->LayerOffset[i].NumLayerRectangles;

		polyOff2 += 4 * roomData->LayerOffset[i].NumLayerVertices; // todo find what struct this is

		vertOff += sizeof(tr5_room_vertex) * roomData->LayerOffset[i].NumLayerVertices;
	}

	memcpy(room, roomData, sizeof(ROOM_INFO));
}

void ReadRooms()
{
	ReadInt32();

	int numRooms = ReadInt32();
	NumberRooms = numRooms;
	Rooms = (ROOM_INFO*)game_malloc(NumberRooms * sizeof(ROOM_INFO));

	printf("NumRooms: %d\n", numRooms);
	
	for (int i = 0; i < NumberRooms; i++)
	{
		// Ignore XELA
		int xela = ReadInt32();

		// Read room data
		int roomDataSize = ReadInt32();
		byte* roomData = (byte*)game_malloc(roomDataSize);
		ReadBytes(roomData, roomDataSize);

		// Put the room data in the struct
		FixUpRoom(&Rooms[i], (ROOM_INFO*)roomData);
	}
}

void BuildOutsideRoomsTable() 
{
	/*long max_slots = 0;
	AllocT(OutsideRoomOffsets, short, 27 * 27);
	AllocT(OutsideRoomTable, char, 27 * 27 * OUTSIDE_Z);
	memset(OutsideRoomTable, -1, 27 * 27 * OUTSIDE_Z);

	char flipped[256];
	memset(flipped, 0, 255);

	for (int i = 0; i < number_rooms; i++)
	{
		if (room[i].flipped_room != -1)
			flipped[i] = true;
	}

	for (int y = 0; y < 108; y += 4)
	{
		for (int x = 0; x < 108; x += 4)
		{
			for (int i = 0; i < number_rooms; i++)
			{
				const auto r = &room[i];

				if (!flipped[i])
				{
					const int rx = (r->z >> SECTOR(1)) + 1;
					const int ry = (r->x >> SECTOR(1)) + 1;

					int j = 0;

					for (int yl = 0; yl < 4; yl++)
					{
						for (int xl = 0; xl < 4; xl++)
						{
							if ((x + xl) >= rx && (x + xl) < (rx + r->x_size - 2) &&
								(y + yl) >= ry && (y + yl) < (ry + r->y_size - 2))
							{
								j = 1;
								break;
							}
						}
					}

					if (!j)
						continue;

					if (i == 255)
					{
						S_Warn("ERROR : Room 255 fuckeroony - go tell Chris\n");
					}

					char* d = &OutsideRoomTable[OUTSIDE_Z * (x >> 2) + OUTSIDE_Z * (y >> 2) * 27];

					for (int j = 0; j < OUTSIDE_Z; j++)
					{
						if (d[j] == -1)
						{
							d[j] = i;

							if (j > max_slots)
								max_slots = j;

							break;
						}
					}

					if (j == OUTSIDE_Z)
					{
						S_Warn("ERROR : Buffer shittage - go tell Chris\n");
					}
				}
			}
		}
	}
	// todo it's a bit incorrect
	char* s = OutsideRoomTable;

	for (int y = 0; y < 27; y++)
	{
		for (int x = 0; x < 27; x++)
		{
			int z = 0;

			char* d = &OutsideRoomTable[OUTSIDE_Z * x + OUTSIDE_Z * y * 27];

			const int i = 27 * y + x;

			while (d[z] != -1)
				z++;

			if (z == 0)
			{
				OutsideRoomOffsets[i] = -1;
			}
			else if (z == 1)
			{
				OutsideRoomOffsets[i] = *d | 0x8000;
			}
			else
			{
				char* p = OutsideRoomTable;

				while (p < s)
				{
					if (memcmp(p, d, z) == 0)
					{
						OutsideRoomOffsets[i] = p - OutsideRoomTable;
						break;
					}
					else
					{
						int z2 = 0;

						while (p[z2] != -1)
							z2++;

						p += z2 + 1;
					}
				}

				if (p >= s)
				{
					OutsideRoomOffsets[i] = s - OutsideRoomTable;

					while (z-- > 0)
						* s++ = *d++;

					*s++ = -1;
				}
			}
		}
	}*/
}
 
void LoadRooms()
{
	printf("LoadRooms\n");
	
	Wibble = 0;
	//RoomLightsCount = 0;
	//Unk_007E7FE8 = 0;

	ReadRooms();
	BuildOutsideRoomsTable();

	int numFloorData = ReadInt32(); 
	FloorData = (short*)game_malloc(numFloorData * 2);
	ReadBytes(FloorData, numFloorData * 2);
}

void FreeLevel()
{
	malloc_ptr = malloc_buffer;
	malloc_free = malloc_size;
	g_Renderer->FreeRendererData();
	g_GameScript->FreeLevelScripts();
}

size_t ReadFileEx(void* ptr, size_t size, size_t count, FILE* stream)
{
	_lock_file(stream);
	size_t result = fread(ptr, size, count, stream);
	_unlock_file(stream);
	return result;
}

void LoadSoundEffects()
{
	NumberSoundSources = ReadInt32();
	if (NumberSoundSources)
	{
		SoundSources = (OBJECT_VECTOR*)game_malloc(NumberSoundSources * sizeof(OBJECT_VECTOR));
		ReadBytes(SoundSources, NumberSoundSources * sizeof(OBJECT_VECTOR));
	}
}

void LoadAnimatedTextures()
{
	NumAnimatedTextures = ReadInt32();
	
	AnimTextureRanges = (short*)game_malloc(NumAnimatedTextures * sizeof(short));
	ReadBytes(AnimTextureRanges, NumAnimatedTextures * sizeof(short));
	
	nAnimUVRanges = ReadInt8();
}

void LoadTextureInfos()
{
	ReadInt32(); // TEX/0
	
	NumObjectTextures = ReadInt32();
	ObjectTextures = (OBJECT_TEXTURE*)game_malloc(NumObjectTextures * sizeof(OBJECT_TEXTURE));

	if (NumObjectTextures > 0)
	{
		for (int i = 0; i < NumObjectTextures; i++)
		{
			tr4_object_texture srctext;
			ReadBytes(&srctext, sizeof(tr4_object_texture));

			OBJECT_TEXTURE* texture = &ObjectTextures[i];

			texture->attribute = srctext.Attribute;
			texture->tileAndFlag = srctext.TileAndFlag; // &0x7FFF;
			texture->newFlags = texture->newFlags; // srctext.TileAndFlag ^ (srctext.TileAndFlag ^ srctext.NewFlags) & 0x7FFF;

			for (int j = 0; j < 4; j++)
			{
				texture->vertices[j].x = srctext.Vertices[j].Xpixel / 256.0f;
				texture->vertices[j].y = srctext.Vertices[j].Ypixel / 256.0f;
			}

			// Adjust UV
			float fx = 1.0f / 256.0f;
			float fy = 1.0f / 256.0f;

			int correction = texture->newFlags & 7;

			if (texture->tileAndFlag & 0x8000)
			{
				if (correction == 1)
				{
					texture->vertices[0].x = texture->vertices[0].x - fx;
					texture->vertices[0].y = texture->vertices[0].y + fy;
					texture->vertices[1].x = texture->vertices[1].x + fx;
					texture->vertices[1].y = texture->vertices[1].y + fy;
					texture->vertices[2].x = texture->vertices[2].x + fx;
					texture->vertices[2].y = texture->vertices[2].y - fy;
					texture->vertices[3].x = texture->vertices[3].x - fx;
					texture->vertices[3].y = texture->vertices[3].y - fy;
				}
				else
				{
					texture->vertices[0].x = texture->vertices[0].x + fx;
					texture->vertices[0].y = texture->vertices[0].y + fy;
					texture->vertices[1].x = texture->vertices[1].x - fx;
					texture->vertices[1].y = texture->vertices[1].y + fy;
					texture->vertices[2].x = texture->vertices[2].x - fx;
					texture->vertices[2].y = texture->vertices[2].y - fy;
					texture->vertices[3].x = texture->vertices[3].x + fx;
					texture->vertices[3].y = texture->vertices[3].y - fy;
				}
			}
			else
			{
				switch (correction)
				{
				case 0:
					texture->vertices[0].x = texture->vertices[0].x + fx;
					texture->vertices[0].y = texture->vertices[0].y + fy;
					texture->vertices[1].x = texture->vertices[1].x - fx;
					texture->vertices[1].y = texture->vertices[1].y + fy;
					texture->vertices[2].x = texture->vertices[2].x + fx;
					texture->vertices[2].y = texture->vertices[2].y - fy;
					break;

				case 1:
					texture->vertices[0].x = texture->vertices[0].x - fx;
					texture->vertices[0].y = texture->vertices[0].y + fy;
					texture->vertices[1].x = texture->vertices[1].x - fx;
					texture->vertices[1].y = texture->vertices[1].y - fy;
					texture->vertices[2].x = texture->vertices[2].x - fx;
					texture->vertices[2].y = texture->vertices[2].y + fy;
					break;

				case 2:
					texture->vertices[0].x = texture->vertices[0].x - fx;
					texture->vertices[0].y = texture->vertices[0].y - fy;
					texture->vertices[1].x = texture->vertices[1].x + fx;
					texture->vertices[1].y = texture->vertices[1].y - fy;
					texture->vertices[2].x = texture->vertices[2].x - fx;
					texture->vertices[2].y = texture->vertices[2].y + fy;
					break;

				case 3:
					texture->vertices[0].x = texture->vertices[0].x + fx;
					texture->vertices[0].y = texture->vertices[0].y - fy;
					texture->vertices[1].x = texture->vertices[1].x + fx;
					texture->vertices[1].y = texture->vertices[1].y + fy;
					texture->vertices[2].x = texture->vertices[2].x - fx;
					texture->vertices[2].y = texture->vertices[2].y - fy;
					break;

				case 4:
					texture->vertices[0].x = texture->vertices[0].x - fx;
					texture->vertices[0].y = texture->vertices[0].y + fy;
					texture->vertices[1].x = texture->vertices[1].x + fx;
					texture->vertices[1].y = texture->vertices[1].y + fy;
					texture->vertices[2].x = texture->vertices[2].x - fx;
					texture->vertices[2].y = texture->vertices[2].y - fy;
					break;

				case 5:
					texture->vertices[0].x = texture->vertices[0].x + fx;
					texture->vertices[0].y = texture->vertices[0].y + fy;
					texture->vertices[1].x = texture->vertices[1].x + fx;
					texture->vertices[1].y = texture->vertices[1].y - fy;
					texture->vertices[2].x = texture->vertices[2].x - fx;
					texture->vertices[2].y = texture->vertices[2].y + fy;
					break;

				case 6:
					texture->vertices[0].x = texture->vertices[0].x + fx;
					texture->vertices[0].y = texture->vertices[0].y - fy;
					texture->vertices[1].x = texture->vertices[1].x - fx;
					texture->vertices[1].y = texture->vertices[1].y - fy;
					texture->vertices[2].x = texture->vertices[2].x + fx;
					texture->vertices[2].y = texture->vertices[2].y + fy;
					break;

				case 7:
					texture->vertices[0].x = texture->vertices[0].x - fx;
					texture->vertices[0].y = texture->vertices[0].y - fy;
					texture->vertices[1].x = texture->vertices[1].x - fx;
					texture->vertices[1].y = texture->vertices[1].y + fy;
					texture->vertices[2].x = texture->vertices[2].x + fx;
					texture->vertices[2].y = texture->vertices[2].y - fy;
					break;

				}
			}
		}
	}
}

void LoadAIObjects()
{
	nAIObjects = ReadInt32();
	
	if (nAIObjects != 0)
	{
		AIObjects = (AIOBJECT*)game_malloc(nAIObjects * sizeof(AIOBJECT));
		ReadBytes(&AIObjects, nAIObjects * sizeof(AIOBJECT));
	}
}

FILE* FileOpen(const char* fileName)
{
	FILE* ptr = fopen(fileName, "rb");
	return ptr;
}

void FileClose(FILE* ptr)
{
	fclose(ptr);
}

void Decompress(byte* dest, byte* src, unsigned long compressedSize, unsigned long uncompressedSize)
{
	int z_result = uncompress(

		dest,       // destination for the uncompressed
								// data.  This should be the size of
								// the original data, which you should
								// already know.

		&uncompressedSize,  // length of destination (uncompressed)
								// buffer

		src,   // source buffer - the compressed data

		compressedSize);   // length of compressed data in bytes

	switch (z_result)
	{
	case Z_OK:
		printf("***** SUCCESS! *****\n");
		break;

	case Z_MEM_ERROR:
		printf("out of memory\n");
		exit(1);    // quit.
		break;

	case Z_BUF_ERROR:
		printf("output buffer wasn't large enough!\n");
		exit(1);    // quit.
		break;
	}
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}

unsigned CALLBACK LoadLevel(void* data)
{
	printf("LoadLevel\n");

	char* filename = (char*)data;

	LevelDataPtr = NULL;
	LevelFilePtr = NULL;

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
		//LoadDemoData();
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
	/*
	// Load also the new file format
	string t5mFileName = string(filename);
	std::transform(t5mFileName.begin(), t5mFileName.end(), t5mFileName.begin(),
		[](unsigned char c) { return std::tolower(c); });
	replace(t5mFileName, ".trc", ".t5m");
	LevelLoader* loader = new LevelLoader(t5mFileName);
	loader->Load();
	delete loader;
	*/
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
	Lara.Vehicle = -1;
	g_GameScript->AssignItemsAndLara();

	// Level loaded
	IsLevelLoading = false;
	g_Renderer->UpdateProgress(100);

	_endthreadex(1);

	return true;
}

void LoadSamples()
{
	// Legacy soundmap size was 450, for now let's store new soundmap size into NumDemoData field
	SoundMapSize = ReadInt16();

	if (SoundMapSize == 0)
		SoundMapSize = SOUND_LEGACY_SOUNDMAP_SIZE;

	for (int i = 0; i < SoundMapSize; i++)
		SampleLUT[i] = ReadInt16();

	NumSamplesInfos = ReadInt32();
	if (NumSamplesInfos)
	{
		SampleInfo = (SAMPLE_INFO*)game_malloc(NumSamplesInfos * sizeof(SAMPLE_INFO));
		ReadBytes(SampleInfo, NumSamplesInfos * sizeof(SAMPLE_INFO));

		int numSampleIndices = ReadInt32();
		if (numSampleIndices)
		{
			int numSamples = 0;
			ReadFileEx(&numSamples, 1, 4, LevelFilePtr);
			//if (feof(LevelFilePtr))
			//	return;

			if (numSamples <= 0)
				return;

			int uncompressedSize;
			int compressedSize;
			char* buffer = (char*)malloc(1048576);

			for (int i = 0; i < numSamples; i++)
			{
				ReadFileEx(&uncompressedSize, 4, 1, LevelFilePtr);
				ReadFileEx(&compressedSize, 4, 1, LevelFilePtr);
				ReadFileEx(buffer, 1, compressedSize, LevelFilePtr);
				Sound_LoadSample(buffer, compressedSize, uncompressedSize, i);
			}

			free(buffer);
		}
	}
	else
	{
		//Log(1, aNoSampleInfos);
	}
}

void LoadBoxes()
{
	// Read boxes
	NumberBoxes = ReadInt32();
	Boxes = (BOX_INFO*)game_malloc(NumberBoxes * sizeof(BOX_INFO));
	ReadBytes(Boxes, NumberBoxes * sizeof(BOX_INFO));

	// Read overlaps
	NumberOverlaps = ReadInt32();
	Overlaps = (short*)game_malloc(NumberOverlaps * sizeof(short));
	ReadBytes(Overlaps, NumberOverlaps * sizeof(short));

	// Read zones
	for (int i = 0; i < 2; i++)
	{
		// Ground zones
		for (int j = 0; j < 4; j++)
		{
			short* zone = (short*)game_malloc(NumberBoxes * sizeof(short));
			ReadBytes(zone, NumberBoxes * sizeof(short));
			Zones[j][i] = zone;
		}

		// Fly zone
		short* zone = (short*)game_malloc(NumberBoxes * sizeof(short));
		ReadBytes(zone, NumberBoxes * sizeof(short));
		Zones[4][i] = zone;
	}

	// By default all blockable boxes are blocked
	for (int i = 0; i < NumberBoxes; i++)
	{
		if (Boxes[i].overlapIndex & BLOCKABLE)
		{
			Boxes[i].overlapIndex |= BLOCKED;
		}
	}
}

int S_LoadLevelFile(int levelIndex)
{
	//DB_Log(2, "S_LoadLevelFile - DLL");
	printf("S_LoadLevelFile\n");
	 
	SOUND_Stop();
	Sound_FreeSamples();
	if (!g_FirstLevel)
		FreeLevel();
	g_FirstLevel = false;
	
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
	//DB_Log(2, "LoadSprites");

	ReadInt32(); // SPR\0

	g_NumSprites = ReadInt32();

	Sprites = (SPRITE*)game_malloc(g_NumSprites * sizeof(SPRITE));

	for (int i = 0; i < g_NumSprites; i++)
	{
		Sprites[i].tile = ReadInt16() + 1;
		Sprites[i].x = ReadInt8();
		Sprites[i].y = ReadInt8();
		Sprites[i].width = ReadInt16();
		Sprites[i].height = ReadInt16();
		Sprites[i].left = (ReadInt16() + 1) / 256.0f;
		Sprites[i].top = (ReadInt16() + 1) / 256.0f;
		Sprites[i].right = (ReadInt16() - 1) / 256.0f;
		Sprites[i].bottom = (ReadInt16() - 1) / 256.0f;
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
					&& Objects[item2->objectNumber].isPickup)
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