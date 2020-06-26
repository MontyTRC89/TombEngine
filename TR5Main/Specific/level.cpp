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
using std::vector;
using std::string;
ChunkId* ChunkTriggersList = ChunkId::FromString("Tr5Triggers");
ChunkId* ChunkTrigger = ChunkId::FromString("Tr5Trigger");
ChunkId* ChunkLuaIds = ChunkId::FromString("Tr5LuaIds");
ChunkId* ChunkLuaId = ChunkId::FromString("Tr5LuaId");

extern GameScript* g_GameScript;

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
vector<OBJECT_TEXTURE> ObjectTextures;
ITEM_INFO* Items;
int LevelItems;
std::vector<ROOM_INFO> Rooms;
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

vector<TEXTURE> RoomTextures;
vector<TEXTURE> MoveablesTextures;
vector<TEXTURE> StaticsTextures;
vector<TEXTURE> SpritesTextures;
TEXTURE MiscTextures;

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

unsigned short ReadUInt16()
{
	unsigned short value = *(unsigned short*)LevelDataPtr;
	LevelDataPtr += 2;
	return value;
}

int ReadInt32()
{
	int value = *(int*)LevelDataPtr;
	LevelDataPtr += 4;
	return value;
}

float ReadFloat()
{
	float value = *(float*)LevelDataPtr;
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

	for (int r = 0; r < Rooms.size(); r++)
	{
		MESH_INFO* mesh = Rooms[r].mesh.data();

		for (int m = 0; m < Rooms[r].mesh.size(); m++)
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

	int numTextures;
	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.size, 1, 4, LevelFilePtr);
		texture.data.reserve(texture.size);
		byte* buffer = (byte*)malloc(texture.size);
		ReadFileEx(buffer, 1, texture.size, LevelFilePtr);
		std::copy(buffer, buffer + texture.size, std::back_inserter(texture.data));
		free(buffer);

		RoomTextures.push_back(texture);
	}

	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.size, 1, 4, LevelFilePtr);
		texture.data.reserve(texture.size);
		byte* buffer = (byte*)malloc(texture.size);
		ReadFileEx(buffer, 1, texture.size, LevelFilePtr);
		std::copy(buffer, buffer + texture.size, std::back_inserter(texture.data));
		free(buffer);

		MoveablesTextures.push_back(texture);
	}

	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.size, 1, 4, LevelFilePtr);
		texture.data.reserve(texture.size);
		byte* buffer = (byte*)malloc(texture.size);
		ReadFileEx(buffer, 1, texture.size, LevelFilePtr);
		std::copy(buffer, buffer + texture.size, std::back_inserter(texture.data));
		free(buffer);

		StaticsTextures.push_back(texture);
	}

	ReadFileEx(&numTextures, 1, 4, LevelFilePtr);
	for (int i = 0; i < numTextures; i++)
	{
		TEXTURE texture;

		ReadFileEx(&texture.width, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.height, 1, 4, LevelFilePtr);
		ReadFileEx(&texture.size, 1, 4, LevelFilePtr);
		texture.data.reserve(texture.size);
		byte* buffer = (byte*)malloc(texture.size);
		ReadFileEx(buffer, 1, texture.size, LevelFilePtr);
		std::copy(buffer, buffer + texture.size, std::back_inserter(texture.data));
		free(buffer);

		SpritesTextures.push_back(texture);
	}

	ReadFileEx(&MiscTextures.width, 1, 4, LevelFilePtr);
	ReadFileEx(&MiscTextures.height, 1, 4, LevelFilePtr);
	ReadFileEx(&MiscTextures.size, 1, 4, LevelFilePtr);
	MiscTextures.data.reserve(MiscTextures.size);
	byte* buffer = (byte*)malloc(MiscTextures.size);
	ReadFileEx(buffer, 1, MiscTextures.size, LevelFilePtr);
	std::copy(buffer, buffer + MiscTextures.size, std::back_inserter(MiscTextures.data));
	free(buffer);
}

void ReadRooms()
{
	ReadInt32();

	int numRooms = ReadInt32();
	//NumberRooms = numRooms;
	//Rooms = (ROOM_INFO*)game_malloc(NumberRooms * sizeof(ROOM_INFO));
	Rooms.clear();
	
	printf("NumRooms: %d\n", numRooms);
	
	for (int i = 0; i < numRooms; i++)
	{
		ROOM_INFO room;

		room.x = ReadInt32();
		room.y = 0;
		room.z = ReadInt32();
		room.minfloor = ReadInt32();
		room.maxceiling = ReadInt32();

		int numDataWords = ReadInt32();

		short numVertices = ReadInt16();
		room.vertices.reserve(numVertices);
		for (int j = 0; j < numVertices; j++)
		{
			tr5_room_vertex vertex;
			vertex.Vertex.x = ReadInt16();
			vertex.Vertex.y = ReadInt16();
			vertex.Vertex.z = ReadInt16();
			vertex.Colour = ReadInt32();
			ReadInt16();
			room.vertices.push_back(vertex);
		}

		short numQuads = ReadInt16();
		room.quads.reserve(numQuads);
		for (int j = 0; j < numQuads; j++)
		{
			tr4_mesh_face4 poly;
			poly.Vertices[0] = ReadInt16();
			poly.Vertices[1] = ReadInt16();
			poly.Vertices[2] = ReadInt16();
			poly.Vertices[3] = ReadInt16();
			poly.Texture = ReadInt16();
			room.quads.push_back(poly);
		}

		short numTriangles = ReadInt16();
		room.triangles.reserve(numTriangles);
		for (int j = 0; j < numTriangles; j++)
		{
			tr4_mesh_face3 poly;
			poly.Vertices[0] = ReadInt16();
			poly.Vertices[1] = ReadInt16();
			poly.Vertices[2] = ReadInt16();
			poly.Texture = ReadInt16();
			room.triangles.push_back(poly);
		}

		short numPortals = ReadInt16();
		for (int j = 0; j < numPortals; j++)
		{
			ROOM_DOOR door;

			door.room = ReadInt16();
			door.normal.x = ReadInt16();
			door.normal.y = ReadInt16();
			door.normal.z = ReadInt16();
			for (int k = 0; k < 4; k++)
			{
				door.vertices[k].x = ReadInt16();
				door.vertices[k].y = ReadInt16();
				door.vertices[k].z = ReadInt16();
			}

			room.doors.push_back(door);
		}

		room.xSize = ReadInt16();
		room.ySize = ReadInt16();
		room.floor.reserve(room.xSize * room.ySize);
		for (int j = 0; j < room.xSize * room.ySize; j++)
		{
			FLOOR_INFO floor;

			floor.index = ReadInt32();
			floor.box = ReadInt32();
			floor.fx = ReadInt32();
			floor.stopper = ReadInt32();
			floor.pitRoom = ReadInt32();
			floor.floor = ReadInt32();
			floor.skyRoom = ReadInt32();
			floor.ceiling = ReadInt32();
			floor.floorCollision.split = ReadInt32();
			floor.floorCollision.noCollision = ReadInt32();
			floor.floorCollision.planes[0].a = ReadFloat();
			floor.floorCollision.planes[0].b = ReadFloat();
			floor.floorCollision.planes[0].c = ReadFloat();
			floor.floorCollision.planes[1].a = ReadFloat();
			floor.floorCollision.planes[1].b = ReadFloat();
			floor.floorCollision.planes[1].c = ReadFloat();
			floor.ceilingCollision.split = ReadInt32();
			floor.ceilingCollision.noCollision = ReadInt32();
			floor.ceilingCollision.planes[0].a = ReadFloat();
			floor.ceilingCollision.planes[0].b = ReadFloat();
			floor.ceilingCollision.planes[0].c = ReadFloat();
			floor.ceilingCollision.planes[1].a = ReadFloat();
			floor.ceilingCollision.planes[1].b = ReadFloat();
			floor.ceilingCollision.planes[1].c = ReadFloat();

			room.floor.push_back(floor);
		}

		room.ambient.x = ReadFloat();
		room.ambient.y = ReadFloat();
		room.ambient.z = ReadFloat();

		short numLights = ReadInt16();
		room.lights.reserve(numLights);
		for (int j = 0; j < numLights; j++)
		{
			ROOM_LIGHT light;

			light.x = ReadFloat();
			light.y = ReadFloat();
			light.z = ReadFloat();
			light.r = ReadFloat();
			light.g = ReadFloat();
			light.b = ReadFloat();
			light.in = ReadFloat();
			light.out = ReadFloat();
			light.radIn = ReadFloat();
			light.radOut = ReadFloat();
			light.range = ReadFloat();
			light.dx = ReadFloat();
			light.dy = ReadFloat();
			light.dz = ReadFloat();
			light.type = ReadInt8();

			room.lights.push_back(light);
		}
		
		short numStatics = ReadInt16();
		room.mesh.reserve(numStatics);
		for (int j = 0; j < numStatics; j++)
		{
			MESH_INFO mesh;

			mesh.x = ReadInt32();
			mesh.y = ReadInt32();
			mesh.z = ReadInt32();
			mesh.yRot = ReadUInt16();
			mesh.shade = ReadUInt16();
			mesh.flags = ReadUInt16();
			mesh.staticNumber = ReadUInt16();

			room.mesh.push_back(mesh);
		}

		room.flippedRoom = ReadInt16();
		room.flags = ReadInt16();
		room.meshEffect = ReadInt8();
		room.reverbType = ReadInt8();
		room.flipNumber = ReadInt8();

		room.itemNumber = NO_ITEM;
		room.fxNumber = NO_ITEM;

		Rooms.push_back(room);
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
	RoomTextures.clear();
	MoveablesTextures.clear();
	StaticsTextures.clear();
	SpritesTextures.clear();
	ObjectTextures.clear();
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
	
	if (NumObjectTextures > 0)
	{
		for (int i = 0; i < NumObjectTextures; i++)
		{
			OBJECT_TEXTURE texture;
			texture.attribute = ReadInt32();
			texture.tileAndFlag = ReadInt32();
			texture.newFlags = ReadInt32();
			for (int j = 0; j < 4; j++)
			{
				texture.vertices[j].x = ReadFloat();
				texture.vertices[j].y = ReadFloat();
			}
			texture.destination = ReadInt32();
			ObjectTextures.push_back(texture);
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
		Sprites[i].tile = ReadInt32();
		Sprites[i].x1 = ReadFloat();
		Sprites[i].y1 = ReadFloat();
		Sprites[i].x2 = ReadFloat();
		Sprites[i].y2 = ReadFloat();
		Sprites[i].x3 = ReadFloat();
		Sprites[i].y3 = ReadFloat();
		Sprites[i].x4 = ReadFloat();
		Sprites[i].y4 = ReadFloat();
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